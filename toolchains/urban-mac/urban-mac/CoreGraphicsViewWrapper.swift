//
//  CoreGraphicsViewWrapper.swift
//  urban-mac
//
//  Created by Thomas Barbalet on 5/6/25.
//

import SwiftUI
import AppKit

struct CoreGraphicsViewWrapper: NSViewRepresentable {
    func makeNSView(context: Context) -> NSView {
        return CoreGraphicsView()
    }

    func updateNSView(_ nsView: NSView, context: Context) {}
}

@MainActor class CoreGraphicsView: NSView {

    override var isFlipped: Bool { true }

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        setupObservers()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        setupObservers()
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    private func setupObservers() {
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(handleAppWillTerminate),
                                               name: NSApplication.willTerminateNotification,
                                               object: nil)

        NotificationCenter.default.addObserver(self,
                                               selector: #selector(handleWindowWillClose),
                                               name: NSWindow.willCloseNotification,
                                               object: nil)
    }

    @objc private func handleAppWillTerminate(notification: Notification) {
        print("Will handle terminate")
    }

    @objc private func handleWindowWillClose(notification: Notification) {
        print("Window is closing – perform any per-window logic here.")
    }

    override var acceptsFirstResponder: Bool { true }
    override func becomeFirstResponder() -> Bool { true }
    override func resignFirstResponder() -> Bool { true }

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
                shared_keyReceived(Int(localKey), 0)
                return
            }
        }

        if let characters = event.characters {
            let firstChar = characters.first!
            if CharacterSet.letters.contains(firstChar.unicodeScalars.first!) {
                shared_keyReceived(Int(firstChar.unicodeScalars.first!.value), 0)
            }
        }
    }

    override func keyUp(with event: NSEvent) {
        shared_keyUp()
    }

    override func acceptsFirstMouse(for event: NSEvent?) -> Bool {
        return true
    }

    override func mouseDown(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        let value = event.modifierFlags.contains(.control) || event.modifierFlags.contains(.option)
        
        if value {
            shared_mouseOption(1)

        } else {
            shared_mouseOption(0)
        }
        shared_mouseReceived(location.x, bounds.height - location.y, 0)
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
        shared_delta(event.deltaX, event.deltaY, 0)
    }

    override func magnify(with event: NSEvent) {
        shared_zoom(event.magnification, 0)
    }

    override func rotate(with event: NSEvent) {
        shared_rotate(Double(event.rotation), 0)
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)

        if shared_simulation_started() == 0 {
            shared_init(0, UInt.random(in: 0 ..< 4294967295))
        }

        guard let context = NSGraphicsContext.current?.cgContext else {
            return
        }

        context.saveGState()
        let dimY = Int(bounds.height)
        let dimX = Int(bounds.width)

        if shared_simulation_started() == 0 {
            context.restoreGState()
            return
        }

        let colorSpace: CGColorSpace = CGColorSpaceCreateDeviceRGB()
        let optionalDrawRef: CGContext? = CGContext(
            data: shared_draw(0, dimX, dimY, 0),
            width: dimX,
            height: dimY,
            bitsPerComponent: 8,
            bytesPerRow: dimX * 4,
            space: colorSpace,
            bitmapInfo: UInt32(CGBitmapInfo.byteOrder32Big.rawValue | CGImageAlphaInfo.noneSkipFirst.rawValue)
        )

        if let drawRef = optionalDrawRef {
            context.setBlendMode(.normal)
            context.setShouldAntialias(false)
            context.setAllowsAntialiasing(false)
            if let image = drawRef.makeImage() {
                let newRect = bounds
                context.draw(image, in: newRect)
            }
        }

        context.restoreGState()

        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
    }

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        window?.contentResizeIncrements = NSSize(width: 4, height: 4)
    }
}
