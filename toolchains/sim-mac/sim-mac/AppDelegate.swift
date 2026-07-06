/****************************************************************

 AppDelegate.swift

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
import SwiftUI

class AppDelegate: NSObject, NSApplicationDelegate {
    static var isTerminating = false

    var terrainWindow: NSWindow!
    var controlWindow: NSWindow!

    func applicationDidFinishLaunching(_ notification: Notification) {
        openAdditionalWindows()
    }

    func applicationWillTerminate(_ notification: Notification) {
        Self.isTerminating = true

        if shared_simulation_started() != 0 {
            shared_close()
        }
    }

    func openAdditionalWindows() {
        // Terrain Window
        let terrainView = NSHostingView(rootView: ViewWrapper(viewType: Int32(NUM_TERRAIN)))
        terrainView.translatesAutoresizingMaskIntoConstraints = false

        terrainWindow = NSWindow(
            contentRect: NSRect(x: 100, y: 500, width: 600, height: 500),
            styleMask: [.titled, .resizable, .closable, .miniaturizable],
            backing: .buffered,
            defer: false
        )
        terrainWindow.title = "Terrain"
        terrainWindow.contentView = NSView()
        terrainWindow.contentView?.addSubview(terrainView)
        NSLayoutConstraint.activate([
            terrainView.leadingAnchor.constraint(equalTo: terrainWindow.contentView!.leadingAnchor),
            terrainView.trailingAnchor.constraint(equalTo: terrainWindow.contentView!.trailingAnchor),
            terrainView.topAnchor.constraint(equalTo: terrainWindow.contentView!.topAnchor),
            terrainView.bottomAnchor.constraint(equalTo: terrainWindow.contentView!.bottomAnchor)
        ])
        terrainWindow.minSize = NSSize(width: 512, height: 400)
        terrainWindow.contentResizeIncrements = NSSize(width: 4, height: 4)
        terrainWindow.makeKeyAndOrderFront(nil)

        // Control Window
        let controlView = NSHostingView(rootView: ViewWrapper(viewType: Int32(NUM_CONTROL)))
        controlView.translatesAutoresizingMaskIntoConstraints = false

        controlWindow = NSWindow(
            contentRect: NSRect(x: 800, y: 500, width: 400, height: 600),
            styleMask: [.titled, .resizable, .closable, .miniaturizable],
            backing: .buffered,
            defer: false
        )
        controlWindow.title = "Control"
        controlWindow.contentView = NSView()
        controlWindow.contentView?.addSubview(controlView)
        NSLayoutConstraint.activate([
            controlView.leadingAnchor.constraint(equalTo: controlWindow.contentView!.leadingAnchor),
            controlView.trailingAnchor.constraint(equalTo: controlWindow.contentView!.trailingAnchor),
            controlView.topAnchor.constraint(equalTo: controlWindow.contentView!.topAnchor),
            controlView.bottomAnchor.constraint(equalTo: controlWindow.contentView!.bottomAnchor)
        ])
        controlWindow.minSize = NSSize(width: 342, height: 512)
        controlWindow.contentResizeIncrements = NSSize(width: 4, height: 4)
        controlWindow.makeKeyAndOrderFront(nil)
    }
}
