/****************************************************************

 ASSimulationView.swift

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

class ASSimulationView: ASMacView {
    
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

    override func awakeFromNib() {
        
        print("awakeFromNib Sim View")
        
        DispatchQueue.main.async { [weak self] in
            if let localSelf = self {
                localSelf.shared = ASShared(frame: localSelf.bounds, title: localSelf.window?.title ?? "")
                
                print(localSelf.shared.identification)
                
                localSelf.shared.startEverything(headyLifting: localSelf.shared.identification == WINDOW_PROCESSING, window: localSelf.window)
            }
        }
    }
    
    
    func renderTime(_ inOutputTime: UnsafePointer<CVTimeStamp>) -> CVReturn {
        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
        return kCVReturnSuccess
    }

    func loadUrlString(_ urlString: String) {
        if let url = URL(string: urlString) {
            NSWorkspace.shared.open(url)
        }
    }

    func debugOutput() {
        DispatchQueue.main.async {
            let panel = self.uniformSavePanel()
            NSLog("Abtaining debug output")
            panel.begin { result in
                if result == .OK {
                    self.shared.scriptDebugHandle(panel.url?.path ?? "")
                }
            }
        }
    }

    // MARK: - IB Actions

    func menuCheckMark(_ sender: Any?, check value: Int) {
        if let menuItem = sender as? NSMenuItem {
            menuItem.state = value != 0 ? .on : .off
        }
    }

    @IBAction func menuFileNew(_ sender: Any?) {
        shared.newSimulation()
        NSLog("Finished new landscape")
    }

    @IBAction func menuFileNewAgents(_ sender: Any?) {
        shared.newAgents()
        NSLog("Finished new agents")
    }

    @IBAction func menuFileOpen(_ sender: Any?) {
        let panel = uniformOpenPanel()
        panel.begin { result in
            if result == .OK {
                if !(self.shared.openFileName(panel.url?.path ?? "", isScript: false)) {
                    NSSound(named: "Pop")?.play()
                }
            }
        }
    }

    @IBAction func menuFileOpenScript(_ sender: Any?) {
        let panel = uniformOpenPanel()
        panel.begin { result in
            if result == .OK {
                if !(self.shared.openFileName(panel.url?.path ?? "", isScript: true)) {
                    NSSound(named: "Pop")?.play()
                }
            }
        }
    }

    @IBAction func menuFileSaveAs(_ sender: Any?) {
        let panel = uniformSavePanel()
        panel.begin { result in
            if result == .OK {
                self.shared.savedFileName(panel.url?.path ?? "")
            }
        }
    }

    @IBAction func menuControlPause(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuPause())
    }

    @IBAction func menuControlFollow(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuFollow())
    }

    @IBAction func menuControlSocialWeb(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuSocialWeb())
    }

    @IBAction func menuControlPrevious(_ sender: Any?) {
        shared.menuPreviousApe()
    }

    @IBAction func menuControlNext(_ sender: Any?) {
        shared.menuNextApe()
    }

    @IBAction func menuControlClearErrors(_ sender: Any?) {
        shared.menuClearErrors()
    }

    @IBAction func menuControlNoTerritory(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuNoTerritory())
    }

    @IBAction func menuControlNoWeather(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuNoWeather())
    }

    @IBAction func menuControlNoBrain(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuNoBrain())
    }

    @IBAction func menuControlNoBrainCode(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuNoBrainCode())
    }

    @IBAction func menuControlDaylightTide(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuDaylightTide())
    }

    @IBAction func menuControlFlood(_ sender: Any?) {
        shared.menuFlood()
    }

    @IBAction func menuControlHealthyCarrier(_ sender: Any?) {
        shared.menuHealthyCarrier()
    }

    @IBAction func menuCommandLine(_ sender: Any?) {
        shared.menuCommandLineExecute()
    }

    @IBAction func loadManual(_ sender: Any?) {
        loadUrlString("https://apesdk.com/doc/man/")
    }

    @IBAction func loadSimulationPage(_ sender: Any?) {
        loadUrlString("https://apesdk.com/")
    }
}
