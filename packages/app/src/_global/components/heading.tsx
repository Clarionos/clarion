import React from "react";

interface Props {
    children: React.ReactNode;
    size?: 1 | 2 | 3 | 4;
    className?: string;
}

export const Heading = ({ children, size, className }: Props) => {
    let element = "";
    let textSize = "";

    switch (size) {
        case 2:
            element = "h2";
            textSize = "text-2xl";
            break;
        case 3:
            element = "h3";
            textSize = "text-xl";
            break;
        case 4:
            element = "h4";
            textSize = "text-lg";
            break;
        case 1:
        default:
            element = "h1";
            textSize = "text-3xl";
    }

    const headingClassName = `${textSize} font-medium text-gray-900 ${
        className || ""
    }`;

    return React.createElement(element, {
        className: headingClassName,
        children,
    });
};
