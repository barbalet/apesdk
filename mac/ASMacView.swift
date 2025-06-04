/****************************************************************

 ASMacView.swift

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/


import Cocoa

class ASMacView: NSView {
    var shared: ASShared!

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)

        if shared_simulation_started() != 0 {
            var dimY = Int(dirtyRect.height)
            let dimX = Int(dirtyRect.width)

            shared.cycledo()

            if #available(macOS 14, *) {
                dimY -= 28
            }

            shared.blitCode(dim_x: dimX, dim_y: dimY)
        }
        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        print("awakeFromNib Mac View")
        DispatchQueue.main.async { [weak self] in
            if let localSelf = self {
                localSelf.shared = ASShared(frame: localSelf.bounds, title: "")
                localSelf.shared.startEverything(headyLifting: true, window: localSelf.window)
                
                if let appDelegate = NSApp.delegate as? AppDelegate {
                    appDelegate.add(shared: localSelf.shared)
                }
            }
        }
        
    }

    @IBAction func menuQuit(_ sender: Any?) {
        NSLog("Quit from menu")
        shared.quitProcedure()
    }

    @IBAction func aboutDialog(_ sender: Any?) {
        shared.about()
    }

    override var acceptsFirstResponder: Bool {
        return true
    }

    override func becomeFirstResponder() -> Bool {
        return true
    }

    override func resignFirstResponder() -> Bool {
        return true
    }

    override func keyDown(with event: NSEvent) {
        var localKey: UInt = 0

        if event.modifierFlags.contains(.control) || event.modifierFlags.contains(.option) {
            localKey = 2048
        }

        if event.modifierFlags.contains(.numericPad) {
            if let theArrow = event.charactersIgnoringModifiers, let keyChar = theArrow.unicodeScalars.first {
                switch keyChar {
                case UnicodeScalar(NSLeftArrowFunctionKey)!:
                    localKey += 28
                case UnicodeScalar(NSRightArrowFunctionKey)!:
                    localKey += 29
                case UnicodeScalar(NSUpArrowFunctionKey)!:
                    localKey += 30
                case UnicodeScalar(NSDownArrowFunctionKey)!:
                    localKey += 31
                default:
                    break
                }
                shared.keyReceived(Int(localKey))
                return
            }
        }

        if let characters = event.characters {
            let firstChar = characters.first!
            if CharacterSet.letters.contains(firstChar.unicodeScalars.first!) {
                shared.keyReceived(Int(firstChar.unicodeScalars.first!.value))
            }
        }
    }

    override func keyUp(with event: NSEvent) {
        shared.keyUp()
    }

    override func acceptsFirstMouse(for event: NSEvent?) -> Bool {
        return true
    }

    override func mouseDown(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        shared.mouseOption(event.modifierFlags.contains(.control) || event.modifierFlags.contains(.option))
        shared.mouseReceived(withXLocation: location.x, yLocation: bounds.height - location.y)
    }

    override func rightMouseDown(with event: NSEvent) {
        mouseDown(with: event)
        shared.mouseOption(true)
    }

    override func otherMouseDown(with event: NSEvent) {
        rightMouseDown(with: event)
    }

    override func mouseUp(with event: NSEvent) {
        shared.mouseUp()
    }

    override func rightMouseUp(with event: NSEvent) {
        mouseUp(with: event)
    }

    override func otherMouseUp(with event: NSEvent) {
        mouseUp(with: event)
    }

    override func mouseDragged(with event: NSEvent) {
        mouseDown(with: event)
    }

    override func rightMouseDragged(with event: NSEvent) {
        rightMouseDown(with: event)
    }

    override func otherMouseDragged(with event: NSEvent) {
        rightMouseDown(with: event)
    }

    override func scrollWheel(with event: NSEvent) {
        shared.delta_x(event.deltaX, delta_y: event.deltaY)
    }

    override func magnify(with event: NSEvent) {
        shared.zoom(event.magnification)
    }

    override func rotate(with event: NSEvent) {
        shared.rotation(Double(event.rotation))
    }
}
