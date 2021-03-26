import React, { useEffect } from "react";

import { SignupForm } from "../components";
import { Heading, Link, Text, useClarion } from "../../_global";

export const AuthPage = () => {
    const { context } = useClarion();

    useEffect(() => {
        if (!context) {
            return;
        }

        if (!context.instance) {
            console.info("wasm not loaded");
            return;
        }
        (context.instance.exports.test as Function)();
    }, [context]);

    return (
        <>
            <div className="min-h-screen px-5 py-24 mx-auto flex bg-gray-50 justify-center md:justify-around">
                <div className="max-w-md bg-white rounded-lg p-8 flex flex-col w-full mt-10 md:mt-0 shadow-md">
                    <Heading size={4} className="mb-4">
                        Welcome to ClarionOS
                    </Heading>
                    <Text className="mb-4">
                        Please provide the following information to setup a
                        brand new account for you
                    </Text>
                    <SignupForm
                        onSubmit={(data) =>
                            alert(
                                "todo: implement signup submission >>> " +
                                    JSON.stringify(data)
                            )
                        }
                    />
                    <Link href="#" className="mt-4 text-sm">
                        Already have an account?
                    </Link>
                </div>
                <div className="hidden md:inline-flex max-w-sm">
                    <img
                        src="/assets/clarion-square-transparent.png"
                        className="object-contain"
                    />
                </div>
            </div>
        </>
    );
};
