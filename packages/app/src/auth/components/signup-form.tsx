import React from "react";

import { Button, Form, SmallText, useFormFields } from "../../_global";

export interface SignupFormData {
    firstName: string;
    lastName: string;
    generatedPassword: string;
    passwordConfirmation: string;
}

const initialForm: SignupFormData = {
    firstName: "",
    lastName: "",
    generatedPassword: "",
    passwordConfirmation: "",
};

export const SignupForm: React.FC<{
    onSubmit: (data: SignupFormData) => void;
}> = ({ onSubmit }) => {
    const [fields, setFields] = useFormFields(initialForm);

    return (
        <form
            className="mt-8 space-y-6"
            onSubmit={(e) => {
                e.preventDefault();
                onSubmit(fields);
            }}
        >
            <div className="space-y-4 mb-4">
                <Form.LabeledSet
                    label="First Name"
                    htmlFor="firstName"
                    className="mb-2"
                >
                    <Form.Input
                        id="firstName"
                        type="text"
                        required
                        value={fields.firstName}
                        onChange={(e: React.ChangeEvent<HTMLInputElement>) =>
                            setFields(e)
                        }
                    />
                </Form.LabeledSet>
                <Form.LabeledSet label="Last Name" htmlFor="lastName">
                    <Form.Input
                        id="lastName"
                        type="text"
                        required
                        value={fields.lastName}
                        onChange={(e: React.ChangeEvent<HTMLInputElement>) =>
                            setFields(e)
                        }
                    />
                </Form.LabeledSet>
                <div className="flex flex-col space-y-2">
                    <Button color="gray" outline className="mt-6">
                        Generate Password
                    </Button>
                    <SmallText>
                        A secure password will be generated for you. Please
                        store it in a safe place.
                    </SmallText>
                </div>
            </div>
            <Form.LabeledSet
                label="Confirm your generated Password"
                htmlFor="password"
            >
                <Form.Input
                    id="password"
                    type="password"
                    placeholder="•••••••••••••••••••••••••••••••••••••••••••••••••••••••"
                    required
                    disabled
                />
            </Form.LabeledSet>
            <Button color="yellow" className="mt-16" isSubmit>
                Confirm
            </Button>
        </form>
    );
};
