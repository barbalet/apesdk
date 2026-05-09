/****************************************************************

 ViewWrapper.swift

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

import SwiftUI

struct ViewWrapper: NSViewRepresentable {
    let viewType: Int32

    func makeNSView(context: Context) -> CustomDrawingView {
        let view = CustomDrawingView(viewType: viewType)
        view.translatesAutoresizingMaskIntoConstraints = false

        // Make sure draw() will be called
        DispatchQueue.main.async {
            if let window = view.window {
                window.contentResizeIncrements = NSSize(width: 4, height: 4)
            }
            view.setNeedsDisplay(view.bounds)
        }

        return view
    }

    func updateNSView(_ nsView: CustomDrawingView, context: Context) {
        nsView.setNeedsDisplay(nsView.bounds)
    }
}
