// exploring pwa functionalities... we need to explore workbox...
// https://webpack.js.org/guides/progressive-web-application/
// https://developers.google.com/web/tools/workbox/guides/using-bundlers

// register service worker
if ("serviceWorker" in navigator) {
    window.addEventListener("load", () => {
        navigator.serviceWorker
            .register("/service-worker.js")
            .then((registration) => {
                console.log("SW registered: ", registration);
            })
            .catch((registrationError) => {
                console.log("SW registration failed: ", registrationError);
            });
    });
}

// Code to handle install prompt on desktop
let deferredPrompt;
const installButton = document.getElementById("install-button");
installButton.style.display = "none";

window.addEventListener("beforeinstallprompt", (e) => {
    // Prevent Chrome 67 and earlier from automatically showing the prompt
    e.preventDefault();
    // Stash the event so it can be triggered later.
    deferredPrompt = e;
    // Update UI to notify the user they can add to home screen
    installButton.style.display = "block";

    installButton.addEventListener("click", () => {
        // hide our user interface that shows our A2HS button
        installButton.style.display = "none";
        // Show the prompt
        deferredPrompt.prompt();
        // Wait for the user to respond to the prompt
        deferredPrompt.userChoice.then((choiceResult) => {
            if (choiceResult.outcome === "accepted") {
                console.log("User accepted the Installation prompt");
            } else {
                console.log("User dismissed the Installation prompt");
            }
            deferredPrompt = null;
        });
    });
});

// Dummy test to make sure that we are loading images even if we are
// offline and operating our PWA
const images = ["fox1", "fox2", "fox3", "fox4"];
const imgElem = document.querySelector("img");

function randomValueFromArray(array) {
    const randomNo = Math.floor(Math.random() * array.length);
    return array[randomNo];
}

setInterval(() => {
    const randomChoice = randomValueFromArray(images);
    imgElem.src = `images/${randomChoice}.jpg`;
}, 2000);
