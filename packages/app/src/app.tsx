import React, { useEffect } from "react";

import { getContext } from "./clarion";

export const App = () => {
    useEffect(() => {
        loadClarionWasm();
    });

    const loadClarionWasm = async () => {
        const clarionContext = await getContext();
        if (!clarionContext.instance) {
            throw new Error("missing clarion instance!!");
        }
        (clarionContext.instance.exports.test as Function)();
    };

    return <div>Hello Clarion React with Tailwind!</div>;
};
