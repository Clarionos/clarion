import React from "react";
import { IconType } from "react-icons/lib";

interface Props {
    children: React.ReactNode;
    color?: string;
    icon?: IconType;
    onClick?: (e: React.MouseEvent<HTMLButtonElement, MouseEvent>) => void;
    href?: string;
    target?: string;
    outline?: boolean;
    isSubmit?: boolean;
    disabled?: boolean;
    className?: string;
}

const BASE_CLASS =
    "items-center px-6 py-2 border rounded-md shadow-sm text-sm font-medium focus:outline-none";

export const Button = ({
    children,
    color,
    icon,
    onClick,
    href,
    target,
    outline,
    isSubmit,
    disabled,
    className,
}: Props) => {
    const buttonColor = color || "yellow";
    const buttonColorSet = outline
        ? ` border-${buttonColor}-300 text-${buttonColor}-600 bg-white hover:bg-${buttonColor}-50`
        : ` border-transparent text-white bg-${buttonColor}-500 hover:bg-${buttonColor}-600`;
    const buttonClass = BASE_CLASS + buttonColorSet + ` ${className || ""}`;

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
        <a href={href} className={buttonClass} target={target}>
            {content}
        </a>
    ) : (
        <button
            onClick={onClick}
            type={isSubmit ? "submit" : "button"}
            className={buttonClass}
            disabled={disabled}
        >
            {content}
        </button>
    );
};

export default Button;
