import { Context } from "@clarionos/bios";

import "./pwa-tests";
import "./websocket-tests";
import { getContext } from "./clarion";

let clarionContext: Context;

const init = async () => {
    clarionContext = await getContext();
};

const startClarionButton = document.getElementById("start-clarion");
startClarionButton.addEventListener("click", () => {
    (clarionContext.instance.exports.test as Function)();
    clarionKvDiv.innerHTML = "loading...";
    setTimeout(printDbStats, 1500);
});

const clarionKvDiv = document.getElementById("clarion-kv");
const printDbStats = () => {
    let db2: any = window.indexedDB.open("foo", 1);
    db2.onsuccess = () => {
        db2 = db2.result;
        let kvStore = db2.transaction("clarion").objectStore("clarion");

        clarionKvDiv.innerHTML = "";
        kvStore.openCursor().onsuccess = (e) => {
            let cursor = e.target.result;

            if (cursor) {
                let key = new TextDecoder().decode(cursor.key);
                let value = new TextDecoder().decode(cursor.value.v);

                let kvEntries = "<pre>";
                kvEntries += `${key} -> ${value}\n`;
                kvEntries += "</pre>";

                clarionKvDiv.innerHTML += kvEntries;
                cursor.continue();
            }
        };
    };
};

init();
