#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

Display *display = NULL;

Atom getAtom(char *name) {
    return XInternAtom(display, name, 0);
}

void printClipboardOwner(char *clipboardName) {
    Atom selectionAtom = getAtom(clipboardName);
    Window selectionOwner = XGetSelectionOwner(display, selectionAtom);

    printf("Owner of selection \"%s\" is window with ID %lu.\n\n", clipboardName, selectionOwner);
}

void printSupportedFormats(char *clipboardName) {
    Atom selectionAtom = getAtom(clipboardName);

    int screen = XDefaultScreen(display);
    Window rootWindow = XRootWindow(display, screen);

    // Create a temporary window for the clipboard contents to be stored on.
    // (the contents will be in the STORE_HERE window property)
    printf("Creating temporary window.\n");
    Window temporaryWindow = XCreateSimpleWindow(display, rootWindow, 0, 0, 1, 1, 0, 0, 0);
    Atom temporaryWindowProperty = getAtom("STORE_HERE");

    Atom targets = getAtom("TARGETS");

    printf("Requesting clipboard.\n");
    // Send request for clipboard in "TARGETS" format (special format that will return a list of supported formats).
    XConvertSelection(display, selectionAtom, targets, temporaryWindowProperty, temporaryWindow, CurrentTime);

    // Wait for SelectionNotify event
    XEvent event;
    while (1) {
        printf("Waiting for SelectionNotify event.\n");
        XNextEvent(display, &event);

        if (event.type == SelectionNotify) {
            printf("Got SelectionNotify!\n");
            XSelectionEvent selectionEvent = *(XSelectionEvent*) &event.xselection;

            // If property is set to None, that means the selection owner couldn't convert the contents.
            if (selectionEvent.property == None) {
                printf("Something went wrong: selection owner couldn't display available targets.\n");
            } else {
                // Display available targets.
                Atom someAtom;
                int actualFormat;
                unsigned long actualItemCount, bytesLeftToRead;
                unsigned char *data = NULL;

                printf("Getting window property.\n");
                XGetWindowProperty(
                        display, temporaryWindow, temporaryWindowProperty,
                        0, sizeof(Atom) * 64, False, XA_ATOM, &someAtom,
                        &actualFormat, &actualItemCount, &bytesLeftToRead,
                        &data
                );

                printf("\nAvailable targets:  \n");
                Atom *availableTargets = (Atom*) data;
                for (int i = 0; i < actualItemCount; i++) {
                    char *atomName = XGetAtomName(display, availableTargets[i]);

                    printf("  - %s\n", atomName);

                    XFree(atomName);
                }

                XFree(data);
            }

            break;
        }
    }

    printf("\nDeleting property and temporary window.\n");
    XDeleteProperty(display, temporaryWindow, temporaryWindowProperty);
    XDestroyWindow(display, temporaryWindow);
}

int main() {
    // Connect to the X11 server.
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        perror("Could not open X11 display! Verify your X11 configuration.\n");
        exit(1);
    }
    printf("==== X11 Selections Inspection ====\n");
    printf("Available actions:\n");
    printf("  O/Owner - display the ID of the window that currently owns the CLIPBOARD selection.\n");
    printf("  F/Formats - current CLIPBOARD selection's available formats.\n");
    printf("  E/Exit - exit the program.\n");
    printf("===================================\n\n");

    // Receive user actions.
    while (1) {
        printf("Enter action [Owner/Formats/Exit]: ");

        char action[1024];
        fgets(action, sizeof(action), stdin);
        for(int i = 0; action[i]; i++){
            action[i] = (char) tolower(action[i]);
        }

        printf("\n");

        if (strcmp(action, "o\n") == 0 || strcmp(action, "owner\n") == 0) {
            printf("=================================================\n");
            printf("         ACTION: show clipboard owner\n");
            printf("=================================================\n");
            printClipboardOwner("CLIPBOARD");
            printf("=================================================\n");
        } else if (strcmp(action, "f\n") == 0 || strcmp(action, "formats\n") == 0) {
            printf("=================================================\n");
            printf("    ACTION: check available clipboard formats\n");
            printf("=================================================\n");
            printSupportedFormats("CLIPBOARD");
            printf("=================================================\n");
        } else if (strcmp(action, "e\n") == 0 || strcmp(action, "exit\n") == 0) {
            printf("Exiting.\n");
            exit(0);
        } else {
            printf("Unsupported action, try again.");
        }
    }

}
