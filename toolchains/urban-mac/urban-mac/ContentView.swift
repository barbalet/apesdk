//
//  ContentView.swift
//  urban-mac
//
//  Created by Thomas Barbalet on 5/6/25.
//

import SwiftUI

struct ContentView: View {
    var body: some View {
        CoreGraphicsViewWrapper()
            .background(Color.white)
            .edgesIgnoringSafeArea(.all)
    }
}
