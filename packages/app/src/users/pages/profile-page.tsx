import React from "react";

import { Form, Heading, Text, storeFile } from "../../_global";

export const ProfilePage = () => {
    const handleProfileImageUpload = async (
        e: React.ChangeEvent<HTMLInputElement>
    ) => {
        e.preventDefault();

        if (!e.target.files || !e.target.files.length) {
            return;
        }

        var file = e.target.files[0];

        // Clear the selection in the file picker input.
        // todo: reset input? (it does not work if the user wants to switch the file)

        // Check if the file is an image.
        if (!file.type.match("image.*")) {
            return alert("You can only select images");
        }

        try {
            const storedObject = await storeFile(file);
            console.info(">>> storedObject: ", storedObject);
        } catch (e) {
            console.error(">>> fail to upload image", e);
            alert(`fail to upload image: ${e.message}`);
        }
    };

    return (
        <>
            <div className="min-h-screen px-5 py-24 mx-auto flex bg-gray-50 justify-center md:justify-around">
                <div className="max-w-md bg-white rounded-lg p-8 flex flex-col w-full mt-10 md:mt-0 relative z-10 shadow-md">
                    <Heading size={4} className="mb-4">
                        Profile Picture
                    </Heading>
                    <Text className="mb-4">
                        Please setup your profile picture. It's super easy, just
                        upload your favorite pic ;)
                    </Text>
                    <Form.FileInput
                        id="profilePic"
                        accept="image/*"
                        label="select an image file"
                        onChange={handleProfileImageUpload}
                    />
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
