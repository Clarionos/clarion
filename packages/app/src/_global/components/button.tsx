import React from "react";
import { IconType } from "react-icons/lib";

interface Props {
    children: React.ReactNode;
    color?: string;
    icon?: IconType;
    onClick?: (e: React.MouseEvent<HTMLButtonElement, MouseEvent>) => void;
    href?: string;
    outline?: boolean;
    isSubmit?: boolean;
}

const BASE_CLASS =
    "inline-flex items-center px-4 py-2 border rounded-md shadow-sm text-sm font-medium focus:outline-none";

export const Button = ({
    children,
    color,
    icon,
    onClick,
    href,
    outline,
    isSubmit,
}: Props) => {
    const buttonColor = color || "indigo";
    const buttonColorSet = outline
        ? ` border-${buttonColor}-300 text-${buttonColor}-700 bg-white hover:bg-${buttonColor}-50`
        : ` border-transparent text-white bg-${buttonColor}-600 hover:bg-${buttonColor}-700`;
    const buttonClass = BASE_CLASS + buttonColorSet;

    const iconOutlineColor = outline ? `text-${color}-500` : "";
    const formattedIcon =
        icon &&
        React.createElement(icon, {
            className: `-ml-1 mr-2 h-5 w-5 ${iconOutlineColor}`,
        });

    const content = (
        <>
            {formattedIcon}
            {children}
        </>
    );

    return href ? (
        <a href={href} className={buttonClass}>
            {content}
        </a>
    ) : (
        <button
            onClick={onClick}
            type={isSubmit ? "submit" : "button"}
            className={buttonClass}
        >
            {content}
        </button>
    );
};

export default Button;
