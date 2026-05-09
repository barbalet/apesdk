/****************************************************************

 SimulationBindings.swift

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

import Cocoa

enum SimulationCycleState {
    case ok
    case quit
    case debugOutput
    case newAgents
}

struct SimulationBindings {
    let identificationForTitle: (String) -> Int
    let start: (Int, UInt) -> Int
    let close: () -> Void
    let simulationStarted: () -> Bool
    let about: () -> Void
    let keyReceived: (Int, Int) -> Void
    let mouseReceived: (Double, Double, Int) -> Void
    let mouseOption: (Bool) -> Void
    let keyUp: () -> Void
    let mouseUp: () -> Void
    let rotate: (Double, Int) -> Void
    let delta: (Double, Double, Int) -> Void
    let zoom: (Double, Int) -> Void
    let draw: (Int, Int, Int, UInt8) -> UnsafeMutablePointer<UInt8>?
    let cycle: (UInt, Int) -> SimulationCycleState
    let maxFPS: (() -> UInt)?
    let newSimulation: ((UInt) -> Int)?
    let newAgents: ((UInt) -> Int)?
    let menu: ((Int) -> Int)?
    let commandLineExecute: (() -> Void)?
    let scriptDebugHandle: ((String) -> Void)?
    let saveFileName: ((String) -> Void)?
    let openFileName: ((String, Bool) -> Bool)?
}

private func randomSimulationSeed() -> UInt {
    UInt.random(in: 0 ..< 4294967295)
}

@MainActor class SimulationShared: NSObject {
    var identification: Int

    private let bindings: SimulationBindings
    private var returnedValue: SimulationCycleState = .ok

    init(frame frameRect: NSRect, title: String, bindings: SimulationBindings) {
        self.bindings = bindings
        identification = bindings.identificationForTitle(title)
        super.init()
    }

    func start() -> Bool {
        let response = bindings.start(identification, randomSimulationSeed())
        if response == -1 {
            return false
        }
        identification = response
        return true
    }

    func simulationStarted() -> Bool {
        bindings.simulationStarted()
    }

    func about() {
        bindings.about()
    }

    func keyReceived(_ key: Int) {
        bindings.keyReceived(key, identification)
    }

    func mouseReceived(withXLocation xLocation: Double, yLocation: Double) {
        bindings.mouseReceived(xLocation, yLocation, identification)
    }

    func mouseOption(_ mouseOption: Bool) {
        bindings.mouseOption(mouseOption)
    }

    func keyUp() {
        bindings.keyUp()
    }

    func mouseUp() {
        bindings.mouseUp()
    }

    func rotation(_ rotationAmount: Double) {
        bindings.rotate(rotationAmount, identification)
    }

    func delta_x(_ deltaX: Double, delta_y: Double) {
        bindings.delta(deltaX, delta_y, identification)
    }

    func zoom(_ zoomAmount: Double) {
        bindings.zoom(zoomAmount, identification)
    }

    func timeInterval() -> TimeInterval {
        guard let maxFPS = bindings.maxFPS?(), maxFPS != 0 else {
            return 1.0 / 60.0
        }
        return 1.0 / TimeInterval(maxFPS)
    }

    func close() {
        bindings.close()
    }

    func newSimulation() {
        _ = bindings.newSimulation?(randomSimulationSeed())
    }

    func newAgents() {
        _ = bindings.newAgents?(randomSimulationSeed())
    }

    func cycle() {
        let timeInfo = UInt(CFAbsoluteTimeGetCurrent())
        returnedValue = bindings.cycle(timeInfo, identification)
    }

    func cycleDebugOutput() -> Bool {
        returnedValue == .debugOutput
    }

    func cycleQuit() -> Bool {
        returnedValue == .quit
    }

    func cycleNewApes() -> Bool {
        returnedValue == .newAgents
    }

    private func menuValue(_ menuValue: Int) -> Int {
        bindings.menu?(menuValue) ?? 0
    }

    func menuPause() -> Int {
        menuValue(NA_MENU_PAUSE)
    }

    func menuFollow() -> Int {
        menuValue(NA_MENU_FOLLOW)
    }

    func menuSocialWeb() -> Int {
        menuValue(NA_MENU_SOCIAL_WEB)
    }

    func menuPreviousApe() {
        _ = bindings.menu?(NA_MENU_PREVIOUS_APE)
    }

    func menuNextApe() {
        _ = bindings.menu?(NA_MENU_NEXT_APE)
    }

    func menuClearErrors() {
        _ = bindings.menu?(NA_MENU_CLEAR_ERRORS)
    }

    func menuNoTerritory() -> Int {
        menuValue(NA_MENU_TERRITORY)
    }

    func menuNoWeather() -> Int {
        menuValue(NA_MENU_WEATHER)
    }

    func menuNoBrain() -> Int {
        menuValue(NA_MENU_BRAIN)
    }

    func menuNoBrainCode() -> Int {
        menuValue(NA_MENU_BRAINCODE)
    }

    func menuDaylightTide() -> Int {
        menuValue(NA_MENU_TIDEDAYLIGHT)
    }

    func menuFlood() {
        _ = bindings.menu?(NA_MENU_FLOOD)
    }

    func menuHealthyCarrier() {
        _ = bindings.menu?(NA_MENU_HEALTHY_CARRIER)
    }

    func menuCommandLineExecute() {
        bindings.commandLineExecute?()
    }

    func scriptDebugHandle(_ fileName: String) {
        bindings.scriptDebugHandle?(fileName)
    }

    func savedFileName(_ name: String) {
        bindings.saveFileName?(name)
    }

    func openFileName(_ name: String, isScript scriptFile: Bool) -> Bool {
        bindings.openFileName?(name, scriptFile) ?? false
    }

    func sharedId() -> Int {
        identification
    }

    func blitCode(dim_x: size_t, dim_y: size_t) {
        guard
            let context = NSGraphicsContext.current?.cgContext,
            let drawBuffer = bindings.draw(sharedId(), Int(dim_x), Int(dim_y), 0)
        else {
            return
        }

        context.saveGState()

        let colorSpace = CGColorSpaceCreateDeviceRGB()
        let optionalDrawRef = CGContext(
            data: UnsafeMutableRawPointer(drawBuffer),
            width: dim_x,
            height: dim_y,
            bitsPerComponent: 8,
            bytesPerRow: dim_x * 4,
            space: colorSpace,
            bitmapInfo: UInt32(CGBitmapInfo.byteOrder32Big.rawValue | CGImageAlphaInfo.noneSkipFirst.rawValue)
        )

        if let drawRef = optionalDrawRef {
            context.setBlendMode(.normal)
            context.setShouldAntialias(false)
            context.setAllowsAntialiasing(false)

            if let image = drawRef.makeImage() {
                let newRect = NSRect(x: 0, y: 0, width: CGFloat(dim_x), height: CGFloat(dim_y))
                context.translateBy(x: 0, y: CGFloat(dim_y))
                context.scaleBy(x: 1, y: -1)
                context.draw(image, in: newRect)
            }
        }

        context.restoreGState()
    }

    func quitProcedure() {
        close()
        exit(0)
    }

    func startEverything(headyLifting: Bool, window: NSWindow?) {
        let increments = NSSize(width: 4, height: 4)
        window?.resizeIncrements = increments
        if headyLifting && start() == false {
            quitProcedure()
            return
        }
        window?.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    func cycledo() {
        cycle()

        if cycleQuit() {
            quitProcedure()
        }

        if cycleNewApes() {
            newAgents()
        }
    }

    func startEverything(_ headyLifting: Bool) {
        if headyLifting && !start() {
            quitProcedure()
            return
        }
        NSApp.activate(ignoringOtherApps: true)
    }
}

@MainActor class SimulationMacView: NSView {
    var shared: SimulationShared!

    func makeShared(frame frameRect: NSRect, title: String) -> SimulationShared {
        fatalError("Subclasses must provide a simulation controller")
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)

        if let shared, shared.simulationStarted() {
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
        DispatchQueue.main.async { [weak self] in
            if let localSelf = self {
                localSelf.shared = localSelf.makeShared(frame: localSelf.bounds, title: "")
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
        true
    }

    override func becomeFirstResponder() -> Bool {
        true
    }

    override func resignFirstResponder() -> Bool {
        true
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
        true
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
