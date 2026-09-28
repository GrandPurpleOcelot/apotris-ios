export const TURN_CREDENTIAL_LIFETIME_SECONDS = 12 * 60 * 60;

export async function createTurnCredentials(secret: string, now = Date.now()) {
    const username = `${Math.floor(now / 1000) + TURN_CREDENTIAL_LIFETIME_SECONDS}:${crypto.randomUUID()}`;
    const key = await crypto.subtle.importKey(
        'raw',
        new TextEncoder().encode(secret),
        { name: 'HMAC', hash: 'SHA-1' },
        false,
        ['sign'],
    );
    const signature = await crypto.subtle.sign('HMAC', key, new TextEncoder().encode(username));
    const password = btoa(String.fromCharCode(...new Uint8Array(signature)));
    return { username, password };
}
