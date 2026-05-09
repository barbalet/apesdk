//
//  urban_macApp.swift
//  urban-mac
//
//  Created by Thomas Barbalet on 5/6/25.
//

import SwiftUI

@main
struct UrbanMacApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
                .frame(minWidth: 1024, minHeight: 768)
        }
        .windowResizability(.contentSize)
    }
}
