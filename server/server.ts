/**
 This file is part of the Apotris multiplayer server implementation.
 Copyright (C) 2024 Isaac Aronson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.
 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// Constants
const HOST_NAME = Deno.env.get("HOST_NAME") || "0.0.0.0";
const SERVER_PORT = Deno.env.get("PORT") ?? 8088;

const MAX_PLAYERS = 5;

const clients: Map<string, WebSocket> = new Map<string, WebSocket>();
const rooms: Map<string, string[]> = new Map<string, string[]>();

const respond = (code: number, data: string, contentType = "text/plain") => {
  return new Response(data, {
    status: code,
    headers: {
      "Content-Type": contentType,
      "Access-Control-Allow-Origin": "*",
    },
  });
};

const getRandomInt = (min: number, max: number) => {
  return Math.floor(Math.random() * (max - min + 1)) + min;
};

const handler = (request: Request): Response => {
  console.info(`${request.method} :: ${request.url}`);

  const upgradeHeader = request.headers.get("Upgrade");
  if (upgradeHeader?.toLowerCase() != "websocket") {
    return respond(404, "Not Found");
  }

  const { socket, response } = Deno.upgradeWebSocket(request);
  const url = new URL(request.url);
  const clientID = url.pathname.split("/")[1];
  const requestedRoomID = url.pathname.split("/")[2];

  console.info(`WS Upgrade :: ${request.url}`);

  socket.addEventListener("open", (_websocketRequest) => {
    clients.set(clientID, socket);
    console.info(`Client connected :: ${clientID}`);

    // Try to match the client with an existing room
    let roomID;
    if (requestedRoomID === "match") {
      for (const [id, room] of rooms.entries()) {
        if (room.length < 2) {
          room.push(clientID);
          roomID = id;
          break;
        }
      }
    } else {
      for (const [id, room] of rooms.entries()) {
        if (room.length >= MAX_PLAYERS) {
          socket.send(JSON.stringify({ type: "full", id: "tryagain" }));
          return;
        }
        if (id.toLowerCase() === requestedRoomID.toLowerCase()) {
          room.push(clientID);
          roomID = id.toLowerCase();
          break;
        }
      }
    }

    // If no room is available, create a new one
    if (!roomID && requestedRoomID != "match") {
      roomID = `${requestedRoomID.toLowerCase()}`;
      rooms.set(roomID, [clientID]);
    } else if (!roomID) {
      roomID = `room-${Math.floor(Math.random() * 100000)}`;
      rooms.set(roomID, [clientID]);
    }

    // If the room is full, send match messages to both clients
    const room = rooms.get(roomID) ?? [];
    if (room.length > 1) {
      const seed = getRandomInt(1, 2000000000);
      room.forEach((clientID, index) => {
        // Send the client's own id and iter
        clients
          .get(clientID)
          ?.send(JSON.stringify({ type: "match", id: clientID, iter: index }));

        // Send the other clients' ids and iters
        room.forEach((otherClientID, otherIndex) => {
          if (clientID !== otherClientID) {
            clients.get(clientID)?.send(
              JSON.stringify({
                type: "match",
                id: otherClientID,
                iter: otherIndex,
              }),
            );
          }
        });

        // Send the shared seed
        clients
          .get(clientID)
          ?.send(JSON.stringify({ type: "seed", id: seed.toString() }));
        console.info(`Sent ${clientID} a seed value of ${seed}`);
      });
      console.log("Room connected!");
    } else {
      // If the room is not full, send an iter message with 0
      socket.send(JSON.stringify({ type: "match", id: clientID, iter: 0 }));
      console.log("Room pending");
    }
  });

  socket.addEventListener("message", (websocketMessage) => {
    if (typeof websocketMessage.data != "string") return;

    console.info(`Client ${clientID} :: << :: ${websocketMessage.data}`);

    try {
      const message = JSON.parse(websocketMessage.data);

      const destinationID = message.id;
      const destination = clients.get(destinationID);

      if (!destination) {
        console.error(`Client not found :: ${destinationID}`);
        return;
      }

      message.id = clientID;

      const data = JSON.stringify(message);

      console.info(`Client ${destinationID} :: >> :: ${data}`);
      destination.send(data);
    } catch (error) {
      console.error(`Invalid JSON :: ${error}`);
    }
  });

  socket.addEventListener("close", () => {
    clients.delete(clientID);

    // Remove the client from their room
    for (const [id, room] of rooms.entries()) {
      const index = room.indexOf(clientID);
      if (index !== -1) {
        delete room[index];

        // If the room is empty, remove it
        if (room.length === 0) {
          rooms.delete(id);
        } else {
          rooms.set(id, Array.from(room.filter((value) => value)));
          // Send a close message to the other clients in the room
          for (const client of rooms.get(id) ?? []) {
            clients
              .get(client)
              ?.send(JSON.stringify({ type: "close", id: clientID }));
            console.info(`Client ${client} heard that ${clientID} quit`);
          }
        }

        break;
      }
    }

    console.info(`Client disconnected :: ${clientID}`);
  });

  return response;
};

const shutdown = () => {
  Deno.exit();
};
// Graceful shutdown of network clients
Deno.addSignalListener("SIGINT", shutdown);
Deno.addSignalListener("SIGTERM", shutdown);

Deno.serve({
  hostname: HOST_NAME,
  port: Number(SERVER_PORT),
  handler,
});
