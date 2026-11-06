# Beach Please Drop Assist (eMAG)

A userscript that runs inside your own logged-in browser on emag.ro. It never clicks a buy or checkout button for you.

- **Countdown on eMAG's clock.** Syncs to eMAG's own server clock using the `Date` header of a handful of tiny same-origin requests. Shows the offset from your clock and its error margin.
- **Reload at the exact moment.** Once you press ARM, it reloads the product page timed by your measured latency, so the request lands about 80 ms after 08:00:00 on eMAG's clock.
- **Limited retries.** If the buy button isn't there yet, it reloads again at a human pace: up to 8 times in the first 25 s. It stops if eMAG shows a human check.
- **Glowing next button.** Highlights and focuses the next button: Adaugă în coș → Vezi detalii coș → Continuă → Plasează comanda. You press Enter or click each one.
- **Stopwatch.** Runs from drop time through checkout.
- **Rehearsal mode.** On any other product page, fakes "08:00" 20 s from now so you can time your checkout.

## Install (2 minutes)

1. Install **Tampermonkey** (Chrome, Edge, Firefox).
   In Chrome: `chrome://extensions` → Tampermonkey → Details → turn on **Allow User Scripts** (or Developer mode).
2. Open the raw file `beach-please-drop.user.js` (GitHub → Raw). Tampermonkey shows an install page → **Install**.
3. Open the drop product page, keep the tab **in front**, press **ARM**.

## Config

At the top of the script: `target` (drop time), `productPath` (the product's `/pd/<code>` path), and the retry settings.
