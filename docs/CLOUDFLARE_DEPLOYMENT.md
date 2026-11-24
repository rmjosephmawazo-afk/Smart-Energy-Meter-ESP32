# Deploy the dashboard with Cloudflare Pages

Cloudflare Pages hosts the static dashboard over HTTPS. The ESP32 connects directly to EMQX using MQTT over TLS; the browser connects to EMQX using WSS.

## Option A — connect GitHub repository

1. Sign in to the Cloudflare dashboard.
2. Open **Workers & Pages** and choose **Create application** → **Pages** → **Connect to Git** (wording may vary).
3. Authorize GitHub and select `Smart-Energy-Meter-ESP32`.
4. Configure the build:
   - Framework preset: **None**
   - Build command: leave blank
   - Build output directory: `frontend`
   - Root directory: repository root
5. Deploy. Cloudflare will provide an HTTPS Pages URL.
6. Open the deployed site, enter your EMQX WSS endpoint and dashboard-specific credentials, and connect.

## Option B — direct upload

1. In Cloudflare **Workers & Pages**, create a Pages project using direct upload.
2. Upload the contents of this repository's `frontend` folder (not the entire repository).
3. Complete deployment and open the HTTPS URL.

## After deployment

- Confirm the page loads over HTTPS.
- Confirm the EMQX WSS listener is enabled and permits the Pages site's browser origin if origin restrictions are configured.
- Check browser developer tools for WebSocket, TLS, authentication, or ACL errors.
- Do not store broker passwords in source files, repository secrets exposed to frontend builds, or public JavaScript. Static frontend variables are not secret.
- Use restricted broker ACLs. For production/multiple users, use a backend authentication service and short-lived scoped credentials rather than shared long-lived browser passwords.

Cloudflare Pages only hosts the dashboard. It does not automatically create the EMQX broker, provision device credentials, or connect the ESP32.
