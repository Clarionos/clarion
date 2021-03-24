import React, { HTMLProps, InputHTMLAttributes } from "react";

export const Label: React.FC<{
    htmlFor: string;
}> = (props) => {
    return (
        <label className="block text-sm font-medium text-gray-700" {...props}>
            {props.children}
        </label>
    );
};

export const Input: React.FC<HTMLProps<HTMLInputElement>> = (props) => {
    return (
        <input
            className="w-full bg-white rounded border border-gray-300 focus:border-yellow-500 focus:ring-2 focus:ring-yellow-200 text-base outline-none text-gray-700 py-1 px-3 leading-8 transition-colors duration-200 ease-in-out"
            {...props}
        />
    );
};

export const FileInput: React.FC<{
    id: string;
    label?: string;
}> = (props) => {
    return (
        <label
            htmlFor={props.id}
            className="relative cursor-pointer bg-white rounded-md font-medium text-yellow-500 hover:text-yellow-400 focus-within:outline-none focus-within:ring-2 focus-within:ring-offset-2 focus-within:ring-yellow-400"
        >
            <span>{props.label || "Attach File..."}</span>
            <input type="file" className="sr-only" {...props} />
        </label>
    );
};

export const Select: React.FC = (props) => {
    return (
        <select
            className="mt-1 block w-full py-2 px-3 border border-gray-300 bg-white rounded-md shadow-sm focus:outline-none focus:ring-indigo-500 focus:border-yellow-500 sm:text-sm"
            {...props}
        >
            {props.children}
        </select>
    );
};

export const TextArea: React.FC<HTMLProps<HTMLTextAreaElement>> = (props) => {
    return (
        <textarea
            rows={3}
            className="w-full bg-white rounded border border-gray-300 focus:border-yellow-500 focus:ring-2 focus:ring-yellow-200 h-32 text-base outline-none text-gray-700 py-1 px-3 resize-none leading-6 transition-colors duration-200 ease-in-out"
            {...props}
        ></textarea>
    );
};

export const LabeledSet: React.FC<{
    htmlFor: string;
    label: string;
    description?: string;
    className?: string;
}> = ({
    htmlFor,
    label,
    children,
    description = undefined,
    className = undefined,
}) => {
    return (
        <div className={className}>
            <Label htmlFor={htmlFor}>{label}</Label>
            <div className="mt-1">{children}</div>
            {description && (
                <p className="mt-2 text-sm text-gray-500">{description}</p>
            )}
        </div>
    );
};

export const Form = {
    Label,
    Input,
    Select,
    TextArea,
    LabeledSet,
    FileInput,
};

export default Form;
