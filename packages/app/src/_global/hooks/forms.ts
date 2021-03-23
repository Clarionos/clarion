import { useState } from "react";

export type SetValuesEvent = (event?: {
    target: { id: string; value: any };
}) => void;

export const useFormFields = <T>(initialState: T): [T, SetValuesEvent] => {
    const [fields, setValues] = useState<T>(initialState);

    return [
        fields,
        (event = undefined) => {
            if (!event) {
                setValues(initialState);
                return;
            }

            setValues({
                ...fields,
                [event.target.id]: event.target.value,
            });
        },
    ];
};
