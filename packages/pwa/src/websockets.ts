const wsUri = "wss://echo.websocket.org/";
const offlineMenu = document.getElementById("offline-menu");
const onlineMenu = document.getElementById("online-menu");
const connectButton = document.getElementById("connect");
const disconnectButton = document.getElementById("disconnect");
const submitButton = document.getElementById("submit");
const output = document.getElementById("output");

let websocket: WebSocket;

const init = () => {
    onlineMenu.style.display = "none";
};

const connectWebSocket = () => {
    writeToScreen("connecting to websocket...");
    websocket = new WebSocket(wsUri);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
    websocket.onerror = onError;
};

const onOpen = (_evt: Event) => {
    writeToScreen(">>> CONNECTED");
    onlineMenu.style.display = "block";
    offlineMenu.style.display = "none";
};

const onClose = (_evt: Event) => {
    writeToScreen("<<< DISCONNECTED");
    onlineMenu.style.display = "none";
    offlineMenu.style.display = "block";
};

const onMessage = (evt: MessageEvent) => {
    writeToScreen(
        '<span style="color: blue;">RESPONSE: ' + evt.data + "</span>"
    );
};

const onError = (evt: Event) => {
    writeToScreen('<span style="color: red;">ERROR:</span> ' + evt);
};

const doSend = (message: string) => {
    writeToScreen("SENT: " + message);
    websocket.send(message);
};

const writeToScreen = (message) => {
    var pre = document.createElement("p");
    pre.style.wordWrap = "break-word";
    pre.innerHTML = message;
    output.appendChild(pre);
};

submitButton.addEventListener("click", () => {
    const message = document.getElementById("message") as HTMLInputElement;
    if (!message.value) {
        return alert("message is blank");
    }

    doSend(message.value);
    message.value = "";
});

connectButton.addEventListener("click", () => {
    connectWebSocket();
});

disconnectButton.addEventListener("click", () => {
    websocket.close();
});

init();
