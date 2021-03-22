import React, { useEffect } from "react";

import { getContext } from "./clarion";

export const App = () => {
    useEffect(() => {
        loadClarionWasm();
    });

    const loadClarionWasm = async () => {
        const clarionContext = await getContext();
        if (!clarionContext.instance) {
            throw new Error("misisng clarion instance");
        }
        (clarionContext.instance.exports.test as Function)();
    };

    return <div>Hello Clarion React</div>;
};

export default App;
