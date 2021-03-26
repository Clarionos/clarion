import React, { useEffect } from "react";
import { BrowserRouter as Router, Switch, Route } from "react-router-dom";

import { getContext } from "./clarion";
import { AuthPage } from "./auth";
import { ProfilePage } from "./users";
import { MemberPage } from "./members";

export const App = () => {
    useEffect(() => {
        loadClarionWasm();
    });

    // todo: implement clarion as a context/hook
    const loadClarionWasm = async () => {
        const clarionContext = await getContext();
        if (!clarionContext.instance) {
            console.info("wasm not loaded");
            return;
            // throw new Error("missing clarion instance!!");
        }
        (clarionContext.instance.exports.test as Function)();
    };

    return (
        <Router>
            <Switch>
                <Route path="/profile">
                    <ProfilePage />
                </Route>
                <Route path="/members">
                    <MemberPage />
                </Route>
                <Route path="/">
                    <AuthPage />
                </Route>
            </Switch>
        </Router>
    );
};
