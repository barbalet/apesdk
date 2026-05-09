/****************************************************************

 CoreGraphicView.c

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

 ****************************************************************/

import AppKit
import SwiftUI

struct CoreGraphicsViewRepresentable: NSViewRepresentable {
    @Binding var commandMode: CommandMode
    let boardWidth: Int
    let boardHeight: Int
    let onMove: (CGPoint, CGSize) -> Void
    let onFace: (CGPoint, CGSize) -> Void
    let onSelectionComplete: () -> Void

    func makeNSView(context: Context) -> NSView {
        let view = CoreGraphicsView(
            frame: NSRect(
                x: 0,
                y: 0,
                width: CGFloat(max(1, boardWidth)),
                height: CGFloat(max(1, boardHeight))
            )
        )
        view.commandMode = commandMode
        view.boardWidth = max(1, boardWidth)
        view.boardHeight = max(1, boardHeight)
        view.onMove = onMove
        view.onFace = onFace
        view.onSelectionComplete = onSelectionComplete
        return view
    }

    func updateNSView(_ nsView: NSView, context: Context) {
        guard let view = nsView as? CoreGraphicsView else { return }
        view.commandMode = commandMode
        view.boardWidth = max(1, boardWidth)
        view.boardHeight = max(1, boardHeight)
        view.onMove = onMove
        view.onFace = onFace
        view.onSelectionComplete = onSelectionComplete
        view.setNeedsDisplay(view.bounds)
    }
}

final class CoreGraphicsView: NSView {
    var commandMode: CommandMode = .select
    var boardWidth: Int = 1024
    var boardHeight: Int = 768
    var onMove: ((CGPoint, CGSize) -> Void)?
    var onFace: ((CGPoint, CGSize) -> Void)?
    var onSelectionComplete: (() -> Void)?

    private var selecting = false
    private var trackingArea: NSTrackingArea?

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        setupView()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        setupView()
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    override var acceptsFirstResponder: Bool { true }

    override func becomeFirstResponder() -> Bool {
        true
    }

    override func acceptsFirstMouse(for event: NSEvent?) -> Bool {
        true
    }

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        window?.makeFirstResponder(self)
    }

    private func setupView() {
        setupTrackingArea()
        setupObservers()
        war_init(0, UInt.random(in: 0 ..< 4294967295))
    }

    private func setupObservers() {
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleAppWillTerminate),
            name: NSApplication.willTerminateNotification,
            object: nil
        )
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleWindowWillClose),
            name: NSWindow.willCloseNotification,
            object: nil
        )
    }

    @objc private func handleAppWillTerminate(notification: Notification) {
        war_close()
    }

    @objc private func handleWindowWillClose(notification: Notification) {
        war_close()
    }

    override func updateTrackingAreas() {
        super.updateTrackingAreas()
        setupTrackingArea()
    }

    private func setupTrackingArea() {
        if let existingTrackingArea = trackingArea {
            removeTrackingArea(existingTrackingArea)
        }
        let options: NSTrackingArea.Options = [
            .mouseMoved,
            .mouseEnteredAndExited,
            .activeInKeyWindow,
            .inVisibleRect
        ]
        let newTrackingArea = NSTrackingArea(
            rect: bounds,
            options: options,
            owner: self,
            userInfo: nil
        )
        trackingArea = newTrackingArea
        addTrackingArea(newTrackingArea)
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)

        guard let context = NSGraphicsContext.current?.cgContext else { return }
        let timeInfo = UInt(CFAbsoluteTimeGetCurrent())
        _ = war_cycle(timeInfo, 0)

        context.saveGState()
        let dimX = max(1, Int(dirtyRect.width))
        let dimY = max(1, Int(dirtyRect.height))
        let colorSpace = CGColorSpaceCreateDeviceRGB()
        let drawBytes = war_draw(0, dimX, dimY, 0)

        if let cgContext = CGContext(
            data: drawBytes,
            width: dimX,
            height: dimY,
            bitsPerComponent: 8,
            bytesPerRow: dimX * 4,
            space: colorSpace,
            bitmapInfo: UInt32(CGBitmapInfo.byteOrder32Big.rawValue | CGImageAlphaInfo.noneSkipFirst.rawValue)
        ) {
            context.setBlendMode(.normal)
            context.setShouldAntialias(false)
            context.setAllowsAntialiasing(false)
            if let image = cgContext.makeImage() {
                context.draw(image, in: dirtyRect)
            }
        }
        context.restoreGState()

        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
    }

    private func boardPoint(from point: CGPoint, in size: CGSize, flipY: Bool) -> CGPoint {
        let width = max(1.0, size.width)
        let height = max(1.0, size.height)
        let boardScaleX = CGFloat(max(1, boardWidth))
        let boardScaleY = CGFloat(max(1, boardHeight))

        let mappedX = (point.x / width) * boardScaleX
        let ySource = flipY ? (height - point.y) : point.y
        let mappedY = (ySource / height) * boardScaleY

        let clampedX = min(max(0.0, mappedX), boardScaleX - 1.0)
        let clampedY = min(max(0.0, mappedY), boardScaleY - 1.0)
        return CGPoint(x: clampedX, y: clampedY)
    }

    private func boardPoint(from event: NSEvent) -> CGPoint {
        let localPoint = convert(event.locationInWindow, from: nil)
        return boardPoint(from: localPoint, in: bounds.size, flipY: true)
    }

    override func mouseDown(with event: NSEvent) {
        selecting = true
        let point = boardPoint(from: event)

        switch commandMode {
        case .select:
            war_mouseReceived(Double(point.x), Double(point.y), 0)
        default:
            break
        }
    }

    override func mouseDragged(with event: NSEvent) {
        guard selecting else { return }
        if commandMode != .select { return }
        let point = boardPoint(from: event)
        war_mouseReceived(Double(point.x), Double(point.y), 0)
    }

    override func mouseUp(with event: NSEvent) {
        guard selecting else { return }
        selecting = false

        let point = boardPoint(from: event)
        let canvas = CGSize(width: CGFloat(boardWidth), height: CGFloat(boardHeight))
        if commandMode == .select {
            war_mouseReceived(Double(point.x), Double(point.y), 0)
            war_mouseUp()
            setNeedsDisplay(bounds)
            onSelectionComplete?()
            return
        }

        switch commandMode {
        case .move:
            onMove?(point, canvas)
        case .face:
            onFace?(point, canvas)
        default:
            break
        }
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        if commandMode == .select && selecting {
            war_mouseUp()
            setNeedsDisplay(bounds)
            onSelectionComplete?()
            selecting = false
        }
    }
}
