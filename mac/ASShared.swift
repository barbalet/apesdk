/****************************************************************

 ASShared.swift

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

import Cocoa

@MainActor class ASShared: SimulationShared {
    convenience init(frame frameRect: NSRect, title: String) {
        self.init(frame: frameRect, title: title, bindings: .ape)
    }
}

private extension SimulationBindings {
    @MainActor static let ape = SimulationBindings(
        identificationForTitle: { title in
            if title == "Terrain" {
                return Int(NUM_TERRAIN)
            }
            if title == "Control" {
                return Int(NUM_CONTROL)
            }
            return Int(NUM_VIEW)
        },
        start: { identification, random in
            Int(shared_init(identification, random))
        },
        close: {
            shared_close()
        },
        simulationStarted: {
            shared_simulation_started() != 0
        },
        about: {
            shared_about()
        },
        keyReceived: { key, identification in
            shared_keyReceived(key, identification)
        },
        mouseReceived: { xLocation, yLocation, identification in
            shared_mouseReceived(xLocation, yLocation, identification)
        },
        mouseOption: { mouseOption in
            shared_mouseOption(mouseOption ? 1 : 0)
        },
        keyUp: {
            shared_keyUp()
        },
        mouseUp: {
            shared_mouseUp()
        },
        rotate: { rotationAmount, identification in
            shared_rotate(rotationAmount, identification)
        },
        delta: { deltaX, deltaY, identification in
            shared_delta(deltaX, deltaY, identification)
        },
        zoom: { zoomAmount, identification in
            shared_zoom(zoomAmount, identification)
        },
        draw: { identification, dimX, dimY, changed in
            shared_draw(identification, dimX, dimY, changed)
        },
        cycle: { timeInfo, identification in
            switch shared_cycle(timeInfo, identification) {
            case SHARED_CYCLE_QUIT:
                return .quit
            case SHARED_CYCLE_DEBUG_OUTPUT:
                return .debugOutput
            case SHARED_CYCLE_NEW_APES:
                return .newAgents
            default:
                return .ok
            }
        },
        maxFPS: {
            UInt(shared_max_fps())
        },
        newSimulation: { seed in
            Int(shared_new(seed))
        },
        newAgents: { seed in
            Int(shared_new_agents(seed))
        },
        menu: { menuValue in
            Int(shared_menu(menuValue))
        },
        commandLineExecute: {
            io_command_line_execution_set()
        },
        scriptDebugHandle: { fileName in
            fileName.withCString {
                shared_script_debug_handle($0)
            }
        },
        saveFileName: { name in
            name.withCString {
                shared_saveFileName($0)
            }
        },
        openFileName: { name, scriptFile in
            name.withCString {
                shared_openFileName($0, scriptFile ? 1 : 0) != 0
            }
        }
    )
}
