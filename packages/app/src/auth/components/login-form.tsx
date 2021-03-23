import React from "react";
import { HiLockClosed } from "react-icons/hi";

import { useFormFields } from "_global/hooks";

const initialForm: LoginFormFields = {
    email: "",
    password: "",
};

export interface LoginFormFields {
    email: string;
    password: string;
}

export const LoginForm = () => {
    const [fields, setFields] = useFormFields(initialForm);

    return (
        <form className="mt-8 space-y-6" onSubmit={(e) => e.preventDefault()}>
            <div className="rounded-md shadow-sm -space-y-px">
                <div>
                    <label htmlFor="email" className="sr-only">
                        Email address
                    </label>
                    <input
                        id="email"
                        type="email"
                        autoComplete="email"
                        className="appearance-none rounded-none relative block w-full px-3 py-2 border border-gray-300 placeholder-gray-500 text-gray-900 rounded-t-md focus:outline-none focus:ring-indigo-500 focus:border-indigo-500 focus:z-10 sm:text-sm"
                        placeholder="Email address"
                        value={fields.email}
                        onChange={setFields}
                        required
                    />
                </div>
                <div>
                    <label htmlFor="password" className="sr-only">
                        Password
                    </label>
                    <input
                        id="password"
                        type="password"
                        autoComplete="current-password"
                        className="appearance-none rounded-none relative block w-full px-3 py-2 border border-gray-300 placeholder-gray-500 text-gray-900 rounded-b-md focus:outline-none focus:ring-indigo-500 focus:border-indigo-500 focus:z-10 sm:text-sm"
                        placeholder="Password"
                        value={fields.password}
                        onChange={setFields}
                        required
                    />
                </div>
            </div>
            <RememberAndForgotPassword />
            <SigninButton />
        </form>
    );
};

const SigninButton = () => (
    <button
        type="submit"
        className="group relative w-full flex justify-center py-2 px-4 border border-transparent text-sm font-medium rounded-md text-white bg-indigo-600 hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-indigo-500"
    >
        <span className="absolute left-0 inset-y-0 flex items-center pl-3">
            <HiLockClosed className="h-5 w-5 text-indigo-500 group-hover:text-indigo-400" />
        </span>
        Sign in
    </button>
);

const RememberAndForgotPassword = () => (
    <div className="flex items-center justify-between">
        <div className="flex items-center">
            <input
                id="remember_me"
                name="remember_me"
                type="checkbox"
                className="h-4 w-4 text-indigo-600 focus:ring-indigo-500 border-gray-300 rounded"
            />
            <label
                htmlFor="remember_me"
                className="ml-2 block text-sm text-gray-900"
            >
                Remember me
            </label>
        </div>

        <div className="text-sm">
            <a
                href="#"
                className="font-medium text-indigo-600 hover:text-indigo-500"
            >
                Forgot your password?
            </a>
        </div>
    </div>
);
