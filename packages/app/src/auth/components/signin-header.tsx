import React from "react";

export const SigninHeader = () => (
    <div className="text-center">
        <img
            className="mx-auto h-12 w-auto"
            src="https://tailwindui.com/img/logos/workflow-mark-indigo-600.svg"
            alt="Workflow"
        />
        <h2 className="mt-6 text-3xl font-extrabold text-gray-900">
            Sign in to your account
        </h2>
        <p className="mt-2 text-sm text-gray-600">
            Or{" "}
            <a
                href="#"
                className="font-medium text-indigo-600 hover:text-indigo-500"
            >
                signup for a new Account
            </a>
        </p>
    </div>
);
