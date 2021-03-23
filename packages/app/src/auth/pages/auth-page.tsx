import React from "react";

import { SigninHeader } from "../components/signin-header";
import { LoginForm } from "../components/login-form";

export const AuthPage = () => {
    return (
        <div className="min-h-screen flex justify-center bg-gray-50 py-12 px-4 sm:px-6 lg:px-8">
            <div className="max-w-md w-full space-y-8">
                <SigninHeader />
                <LoginForm />
            </div>
        </div>
    );
};
