import React from "react";
import { BrowserRouter as Router, Switch, Route } from "react-router-dom";

import { AuthPage } from "./auth";
import { ProfilePage } from "./users";
import { MemberPage } from "./members";

export const App = () => (
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
