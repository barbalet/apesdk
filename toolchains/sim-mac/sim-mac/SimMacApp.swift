/****************************************************************

 SimMacApp.swift

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

@main
struct SimMacApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
    @State private var isPaused = false
    @State private var isTerritory = false
    @State private var isWeather = true
    @State private var isBrain = true
    @State private var isBraincode = false
    
    func uniformOpenPanel() -> NSOpenPanel {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.plainText]
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false
        
        NSLog("Abtaining and returning uniform open panel")
        
        return panel
    }
    
    func uniformSavePanel() -> NSSavePanel {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.plainText]
        panel.nameFieldStringValue = "Untitled"
        panel.canCreateDirectories = true
        NSLog("Abtaining and returning uniform save panel")
        return panel
    }
    
    func loadUrlString(_ urlString: String) {
        if let url = URL(string: urlString) {
            NSWorkspace.shared.open(url)
        }
    }
    
    
    var body: some Scene {
        WindowGroup("View") {
            ViewWrapper(viewType: Int32(NUM_VIEW))
                .frame(width: 512, height: 512)
        }
        .defaultSize(width: 512, height: 512)
        .windowResizability(.contentSize)
        .commands {
            CommandGroup(replacing: .appInfo) {
                Button("About Simulated Ape") {
                    // Your custom About panel logic
                    shared_about()
                }
            }
            
            CommandGroup(replacing: CommandGroupPlacement.newItem) {
                Button("New Simulation") {
                    shared_new(UInt.random(in: 0 ..< 4294967295))
                    NSLog("Finished new landscape")
                }
                .keyboardShortcut("n", modifiers: [.command])
            }
            
            CommandGroup(replacing: .importExport) {
                Button("Open…") {
                    let panel = uniformOpenPanel()
                    panel.begin { result in
                        if result == .OK {
                            if ((shared_openFileName(panel.url?.path ?? "", 0)) == 0) {
                                NSSound(named: "Pop")?.play()
                            }
                        }
                    }
                }
                .keyboardShortcut("o", modifiers: [.command])
                
                Button("Open Script…") {
                    let panel = uniformOpenPanel()
                    panel.begin { result in
                        if result == .OK {
                            if ((shared_openFileName(panel.url?.path ?? "", 1)) == 0) {
                                NSSound(named: "Pop")?.play()
                            }
                        }
                    }
                }
                
                Divider()
                
                Button("Save As…") {
                    let panel = uniformSavePanel()
                    panel.begin { result in
                        if result == .OK {
                            shared_saveFileName(panel.url?.path ?? "")
                        }
                    }
                }
                .keyboardShortcut("s", modifiers: [.command, .shift])
            }
            CommandMenu("Control") {
                Button(isPaused ? "Resume" : "Pause") {
                    let menuResponse = shared_menu(NA_MENU_PAUSE)
                    isPaused = (menuResponse == 1)
                }
                .keyboardShortcut("p", modifiers: [.command])
                Divider()
                Button("Previous Ape", action: { var _ = shared_menu(NA_MENU_PREVIOUS_APE) })
                    .keyboardShortcut("[", modifiers: [.command])
                Button("Next Ape", action: { var _ = shared_menu(NA_MENU_NEXT_APE) })
                    .keyboardShortcut("]", modifiers: [.command])
                Divider()
                Button("Clear Errors", action: { var _ = shared_menu( NA_MENU_CLEAR_ERRORS ) })
                    .keyboardShortcut("e", modifiers: [.command])
                Divider()
                Button(isTerritory ? "No Territory" : "Territory") {
                    let menuResponse = shared_menu(NA_MENU_TERRITORY)
                    isTerritory = (menuResponse == 1)
                }
                Button(isWeather ? "No Weather" : "Weather") {
                    let menuResponse = shared_menu(NA_MENU_WEATHER)
                    isWeather = (menuResponse == 1)
                }
                Button(isBraincode ? "No Braincode" : "Braincode") {
                    let menuResponse = shared_menu(NA_MENU_BRAINCODE)
                    isBraincode = (menuResponse == 1)
                }
                Divider()
                Button("Command Line", action: { io_command_line_execution_set() })
                    .keyboardShortcut("c", modifiers: [.command, .shift])
            }
            
            CommandMenu("Online") {
                Button("Manual") {
                    loadUrlString("https://apesdk.com/doc/man/")
                }
                Button("Simulation Page") {
                    loadUrlString("https://apesdk.com/")
                }
            }
        }
    }
}
