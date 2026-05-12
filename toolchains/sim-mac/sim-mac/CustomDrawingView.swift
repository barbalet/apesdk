/****************************************************************

 CustomDrawingView.swift

 =============================================================

 Copyright 1996-2026 Tom Barbalet. All rights reserved.

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

 ****************************************************************/

import AppKit

@MainActor
class CustomDrawingView: NSView {
    let viewType: Int32
    private var tutorialTrackingArea: NSTrackingArea?
    private var tutorialPointerInside = false

    init(viewType: Int32) {
        self.viewType = viewType
        super.init(frame: .zero)
        self.wantsLayer = false
        self.layer?.backgroundColor = NSColor.windowBackgroundColor.cgColor
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override var isFlipped: Bool { false }

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        window?.acceptsMouseMovedEvents = true
    }

    override func updateTrackingAreas() {
        super.updateTrackingAreas()

        if let tutorialTrackingArea {
            removeTrackingArea(tutorialTrackingArea)
        }

        let options: NSTrackingArea.Options = [
            .activeAlways,
            .enabledDuringMouseDrag,
            .inVisibleRect,
            .mouseEnteredAndExited,
            .mouseMoved
        ]
        let trackingArea = NSTrackingArea(rect: .zero, options: options, owner: self, userInfo: nil)
        addTrackingArea(trackingArea)
        tutorialTrackingArea = trackingArea
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        dirtyRect.fill()
        
        if shared_simulation_started() == 0 {
            shared_init(n_int(self.viewType), UInt.random(in: 0 ..< 4294967295))
        } else {
            guard let context = NSGraphicsContext.current?.cgContext else {
                return
            }
            let time_info : UInt = UInt(CFAbsoluteTimeGetCurrent())
            _ = shared_cycle(time_info, n_int(self.viewType))
            context.saveGState()
            var dimY = Int(dirtyRect.height)
            let dimX = Int(dirtyRect.width)
                        
            if #available(macOS 14, *) {
                dimY -= 28
            }
            let colorSpace: CGColorSpace = CGColorSpaceCreateDeviceRGB()
            let optionalDrawRef: CGContext? = CGContext.init(data: shared_draw(n_int(self.viewType), dimX, dimY, 0), width: dimX, height: dimY, bitsPerComponent: 8, bytesPerRow: dimX * 4, space: colorSpace, bitmapInfo: UInt32(CGBitmapInfo.byteOrder32Big.rawValue | CGImageAlphaInfo.noneSkipFirst.rawValue))
            
            if let drawRef = optionalDrawRef {
                context.setBlendMode(.normal)
                context.setShouldAntialias(false)
                context.setAllowsAntialiasing(false)
                let optionalImage: CGImage? = drawRef.makeImage()
                if let image = optionalImage {
                    let newRect = NSRect(x: 0, y: 0, width: CGFloat(dimX), height: CGFloat(dimY))
                    // Apply vertical flip transformation
                    context.translateBy(x: 0, y: CGFloat(dimY))
                    context.scaleBy(x: 1.0, y: -1.0)

                    context.draw(image, in: newRect)
                }
            }
            context.restoreGState()
        }
        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
    }
    
    override var acceptsFirstResponder: Bool {
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
                shared_keyReceived(Int(localKey), n_int(self.viewType))
                return
            }
        }

        if let characters = event.characters {
            let firstChar = characters.first!
            if CharacterSet.letters.contains(firstChar.unicodeScalars.first!) {
                shared_keyReceived(Int(firstChar.unicodeScalars.first!.value), n_int(self.viewType))
            }
        }
    }

    override func keyUp(with event: NSEvent) {
        shared_keyUp()
    }

    override func acceptsFirstMouse(for event: NSEvent?) -> Bool {
        return true
    }

    override func mouseEntered(with event: NSEvent) {
        noteTutorialPointerEntered()
    }

    override func mouseMoved(with event: NSEvent) {
        noteTutorialPointerEntered()
    }

    override func mouseExited(with event: NSEvent) {
        tutorialPointerInside = false
    }

    override func mouseDown(with event: NSEvent) {
        noteTutorialPointerEntered()
        let location = convert(event.locationInWindow, from: nil)
        let value = event.modifierFlags.contains(.control) || event.modifierFlags.contains(.option)
        if value {
            shared_mouseOption(1)
        } else {
            shared_mouseOption(0)
        }
        shared_mouseReceived(location.x, bounds.height - location.y, n_int(self.viewType))
    }

    override func rightMouseDown(with event: NSEvent) {
        mouseDown(with: event)
        shared_mouseOption(1)
    }

    override func otherMouseDown(with event: NSEvent) {
        rightMouseDown(with: event)
    }

    override func mouseUp(with event: NSEvent) {
        shared_mouseUp()
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
        shared_delta(event.deltaX, event.deltaY, n_int(self.viewType))
    }

    override func magnify(with event: NSEvent) {
        shared_zoom(event.magnification, n_int(self.viewType))
    }

    override func rotate(with event: NSEvent) {
        shared_rotate(Double(event.rotation), n_int(self.viewType))
    }

    private func noteTutorialPointerEntered() {
        if tutorialPointerInside {
            return
        }

        tutorialPointerInside = InitialTutorialController.shared.showNextTutorial(for: self)
    }
    
    private func viewTypeDisplayName(_ type: Int32) -> String {
        switch type {
            case NUM_VIEW: return "View"
            case NUM_TERRAIN: return "Terrain"
            case NUM_CONTROL: return "Control"
        default:
            return "Unknown"
        }
        
        func displayNameViewType(_ type: String) -> Int32 {
            switch type {
                case "View": return NUM_VIEW
                case "Terrain": return NUM_TERRAIN
                case "Control": return NUM_CONTROL
            default:
                return -1
            }
        }
    }
}

#Preview {
    CustomDrawingView(viewType: 1)
}
