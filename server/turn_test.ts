import { createTurnCredentials, TURN_CREDENTIAL_LIFETIME_SECONDS } from './turn.ts';

Deno.test('TURN credentials use coturn time-limited HMAC-SHA1 format', async () => {
    const secret = 'test-secret';
    const credentials = await createTurnCredentials(secret, 1_700_000_000_000);
    const [expiry, id] = credentials.username.split(':');

    if (expiry !== String(1_700_000_000 + TURN_CREDENTIAL_LIFETIME_SECONDS) || !id) {
        throw new Error('Invalid TURN username or expiry');
    }

    const key = await crypto.subtle.importKey(
        'raw',
        new TextEncoder().encode(secret),
        { name: 'HMAC', hash: 'SHA-1' },
        false,
        ['verify'],
    );
    const signature = Uint8Array.from(atob(credentials.password), (char) => char.charCodeAt(0));
    if (!await crypto.subtle.verify('HMAC', key, signature, new TextEncoder().encode(credentials.username))) {
        throw new Error('Invalid TURN password');
    }
});
