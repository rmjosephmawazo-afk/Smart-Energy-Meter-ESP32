# Deploy the dashboard with Cloudflare Pages

Cloudflare Pages hosts the static dashboard over HTTPS. The ESP32 connects directly to EMQX using MQTT over TLS; the browser connects to EMQX using WSS.

## Existing Pages project

A Pages project has been created in the connected Cloudflare account:

- Project: `smart-energy-meter-esp32`
- Site hostname: `smart-energy-meter-esp32.pages.dev`
- Source folder: `frontend/`
- Build command: none (static HTML/CSS/JavaScript)

The Pages project exists, but creating it does not publish a deployment. The GitHub workflow below will publish the site after its required repository secrets are configured.

## Automated deployment from GitHub

The repository includes `.github/workflows/deploy-pages.yml`. It deploys the `frontend/` directory to the Pages project when frontend files change on `main`, or when manually started.

Add these two GitHub Actions repository secrets:

1. Open the repository on GitHub.
2. Go to **Settings → Secrets and variables → Actions → New repository secret**.
3. Add `CLOUDFLARE_ACCOUNT_ID` with your Cloudflare account ID.
4. Add `CLOUDFLARE_API_TOKEN` with a Cloudflare API token that has **Account → Cloudflare Pages → Edit** permission for this account.
5. Do not paste either value into chat, source code, or README files.

Then open **Actions → Deploy dashboard to Cloudflare Pages** and select **Run workflow**, or push a change to `frontend/` on `main`. Wait for the workflow to finish successfully. The published dashboard should then be available at:

https://smart-energy-meter-esp32.pages.dev

If the workflow fails, open its run and read the failed step. Check the token permission, account ID, project name, and that the Pages project is still present.

## Manual Git integration alternative

If you prefer Cloudflare's built-in Git integration, open **Workers & Pages**, select the existing project if available, connect the GitHub repository `rmjosephmawazo-afk/Smart-Energy-Meter-ESP32`, and set:

- Framework preset: **None**
- Build command: leave blank
- Build output directory: `frontend`
- Root directory: repository root
- Production branch: `main`

Avoid configuring both Git integration and the GitHub Actions deployment workflow to publish the same project unless you intentionally want two deployment paths.

## After deployment

- Confirm the page loads over HTTPS.
- Enter the EMQX WSS endpoint and the dashboard-specific MQTT username and password in the dashboard.
- Confirm the EMQX WSS listener is enabled and permits the Pages site's browser origin if origin restrictions are configured.
- Check browser developer tools for WebSocket, TLS, authentication, or ACL errors.
- Do not store broker passwords in source files, repository secrets exposed to frontend builds, or public JavaScript. Static frontend variables are not secret.
- Use restricted broker ACLs. For production/multiple users, use a backend authentication service and short-lived scoped credentials rather than shared long-lived browser passwords.

Cloudflare Pages only hosts the dashboard. It does not automatically create the EMQX broker, provision device credentials, or connect the ESP32.
