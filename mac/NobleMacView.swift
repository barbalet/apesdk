/****************************************************************

 NobleMacView.swift

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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
import MetalKit


//
//private func renderCallback(displayLink: CVDisplayLink?, inNow: CVTimeStamp?, inOutputTime: CVTimeStamp?, flagsIn: CVOptionFlags, flagsOut: UnsafeMutablePointer<CVOptionFlags>?, displayLinkContext: UnsafeMutableRawPointer?) -> CVReturn {
//    return (displayLinkContext as? NobleMacView)?.renderTime(inOutputTime) ?? 0
//}

class NobleMacView: MTKView {
    var drawRef: CGContext?
    var displayLink: CVDisplayLink?

    private var renderer: NobleMTKRenderer?

    var shared: NobleShared?

    func start() {
        
        func renderCallback(displayLink: CVDisplayLink, _ inNow: UnsafePointer<CVTimeStamp>, _ inOutputTime: UnsafePointer<CVTimeStamp>, _ flagsIn: CVOptionFlags, _ flagsOut: UnsafeMutablePointer<CVOptionFlags>, _ displayLinkContext: UnsafeMutableRawPointer) -> CVReturn {
            let view = displayLinkContext as? NobleMacView
            return view?.renderTime(inOutputTime) ?? 0
        }
        
        shared = NobleShared(frame: bounds)

        if let shared = shared {
            print("NobleMacUpdate startView \(shared)")
        }

        let displayID = CGMainDisplayID()
        var error: CVReturn = kCVReturnSuccess
        error = CVDisplayLinkCreateWithCGDisplay(displayID, &displayLink)
        if error != 0 {
            print("DisplayLink created with error:\(error)")
            displayLink = nil
        }
        CVDisplayLinkSetOutputCallback(displayLink!, (renderCallback as! CVDisplayLinkOutputCallback), UnsafeMutablePointer<Void>(unsafeAddressOf(self)))
        CVDisplayLinkStart(displayLink!)
    }

    func sharedReady() {
        // Set the view to use the default device
        device = MTLCreateSystemDefaultDevice()

        if device == nil {
            print("Metal is not supported on this device")
            return
        }

        renderer = NobleMTKRenderer(metalKitView: self, nobleShared: shared!)

        if renderer == nil {
            print("Renderer failed initialization")
            return
        }

        // Initialize our renderer with the view size
        renderer?.mtkView(self, drawableSizeWillChange: drawableSize)

        delegate = renderer
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

    override func awakeFromNib() {
        print("Starting up")
        start()
        startEverything(true)
        sharedReady()
    }

    func startEverything(_ headyLifting: Bool) {
        var increments: NSSize
        increments.height = 4
        increments.width = 4
        window?.contentResizeIncrements = increments
        if headyLifting {
            if shared?.start() == false {
                print("Simulation initialization failed")
                quitProcedure()
                return
            }
        }
        window?.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    func quitProcedure() {
        CVDisplayLinkStop(displayLink!)

        print("Quitting")

        shared?.close()

        print("Quit")

        exit(0)
    }

    @IBAction func menuQuit(_ sender: Any) {
        print("Quit from menu")
        quitProcedure()
    }

    @IBAction func aboutDialog(_ sender: Any) {
        shared?.about("Macintosh INTEL Cocoa")
    }

    override func keyUp(with theEvent: NSEvent) {
        shared?.keyUp()
    }

    override func keyDown(with theEvent: NSEvent) {
        var local_key: Int = 0
        if (theEvent.modifierFlags.rawValue & NSEvent.ModifierFlags.control.rawValue) != 0 || (theEvent.modifierFlags.rawValue & NSEvent.ModifierFlags.option.rawValue) != 0 {
            local_key = 2048
        }

        if theEvent.modifierFlags.rawValue & NSEvent.ModifierFlags.numericPad.rawValue != 0 {
            // arrow keys have this mask
            let theArrow = theEvent.charactersIgnoringModifiers
            var keyChar = unichar(0)
            if (theArrow?.count ?? 0) == 0 {
                return // reject dead keys
            }
            if (theArrow?.count ?? 0) == 1 {
                keyChar = ((theArrow?[(theArrow?.index(theArrow!.startIndex, offsetBy: 0))!])!)
                if keyChar == NSLeftArrowFunctionKey {
                    local_key += 28
                } else if keyChar == NSRightArrowFunctionKey {
                    local_key += 29
                } else if keyChar == NSUpArrowFunctionKey {
                    local_key += 30
                } else if keyChar == NSDownArrowFunctionKey {
                    local_key += 31
                }

                shared?.keyReceived(local_key)
            }
        }
        if theEvent.characters != nil {
            let first: NSRange? = (theEvent.characters as NSString?)?.rangeOfComposedCharacterSequence(at: 0)
            var match: NSRange? = nil
            if let first = first {
                match = (theEvent.characters as NSString?)?.rangeOfCharacter(from: CharacterSet.letters, options: [], range: first)
            }
            if match?.location != NSNotFound {
                let firstChar = ((theEvent.characters?[(theEvent.characters?.index(theEvent.characters!.startIndex, offsetBy: 0))!])!)
                let letters = CharacterSet.letters
                if letters.characterIsMember(firstChar) {
                    // The first character is a letter in some alphabet
                    shared?.keyReceived(firstChar)
                }
            }
        }
    }

    override func mouseDown(with theEvent: NSEvent) {
        let location = convert(theEvent.locationInWindow, from: nil) as? NSPoint
        shared?.mouseOption(((theEvent.modifierFlags.rawValue & NSEvent.ModifierFlags.control.rawValue) != 0 || (theEvent.modifierFlags.rawValue & NSEvent.ModifierFlags.option.rawValue) != 0))
        shared?.mouseReceived(withXLocation: (Double(location!.x)), yLocation: (Double(bounds.size.height - location!.y)))
    }

    override func rightMouseDown(with theEvent: NSEvent) {
        mouseDown(with: theEvent)
        shared?.mouseOption(true)
    }

    override func otherMouseDown(with theEvent: NSEvent) {
        rightMouseDown(with: theEvent)
    }

    override func mouseUp(with theEvent: NSEvent) {
        shared?.mouseUp()
    }

    override func rightMouseUp(with theEvent: NSEvent) {
        mouseUp(with: theEvent)
    }

    override func otherMouseUp(with theEvent: NSEvent) {
        mouseUp(with: theEvent)
    }

    override func mouseDragged(with theEvent: NSEvent) {
        mouseDown(with: theEvent)
    }

    override func scrollWheel(with theEvent: NSEvent) {
        shared?.delta_x(Double(theEvent.deltaX), delta_y: (Double(theEvent.deltaY)))
    }

    override func rightMouseDragged(with theEvent: NSEvent) {
        rightMouseDown(with: theEvent)
    }

    override func otherMouseDragged(with theEvent: NSEvent) {
        rightMouseDown(with: theEvent)
    }

    func renderTime(_ inOutputTime: CVTimeStamp?) -> CVReturn {
        shared?.cycle()

        DispatchQueue.main.async(execute: {
            self.needsDisplay = true
        })
        return kCVReturnSuccess
    }

// MARK: ---- Method Overrides ----

    func acceptsFirstMouse() -> Bool {
        return true
    }

    override func magnify(with event: NSEvent) {
        shared?.zoom((Double(event.magnification)))
    }

    override func rotate(with event: NSEvent) {
        shared?.rotation((Double(event.rotation)))
    }
}

