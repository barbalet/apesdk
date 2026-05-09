/****************************************************************

 ContentView.swift

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
@preconcurrency import Combine
import SwiftUI

enum CommandMode: Int, CaseIterable, Identifiable {
    case select
    case move
    case face

    var id: Int { rawValue }

    var title: String {
        switch self {
        case .select:
            return "Select"
        case .move:
            return "Move"
        case .face:
            return "Set Facing"
        }
    }

    var hint: String {
        switch self {
        case .select:
            return "Drag to select unit groups."
        case .move:
            return "Click destination to move selected groups."
        case .face:
            return "Click direction target for selected groups."
        }
    }
}

enum ArmyFilter: Int, CaseIterable, Identifiable {
    case all
    case red
    case blue

    var id: Int { rawValue }
    var armyCode: Int {
        switch self {
        case .all:
            return Int(WAR_ARMY_ALL)
        case .red:
            return 0
        case .blue:
            return 1
        }
    }

    var title: String {
        switch self {
        case .all:
            return "All Armies"
        case .red:
            return "Army 0"
        case .blue:
            return "Army 1"
        }
    }
}

private enum WarUnitTypeStatCode: Int {
    case defence = 0
    case meleeAttack = 1
    case meleeDamage = 2
    case missileAttack = 4
    case missileDamage = 5
    case missileRange = 8
    case speedMaximum = 9
    case stature = 10
    case woundsPerCombatant = 12
}

struct UnitTypeSnapshot: Identifiable {
    let id: Int
    let name: String
    let defense: Int
    let meleeAttack: Int
    let meleeDamage: Int
    let missileAttack: Int
    let missileDamage: Int
    let speed: Int
    let stature: Int
    let woundsPerCombatant: Int
    let missileRange: Int
}

struct UnitSnapshot: Identifiable {
    let id: Int
    let typeId: Int
    let alignment: Int
    let alive: Int
    let total: Int
    let averageX: Int
    let averageY: Int
    let selected: Bool
    let commandMode: Int

    var commandLabel: String {
        if commandMode == Int(WAR_COMMAND_MOVE) {
            return "Move"
        }
        if commandMode == Int(WAR_COMMAND_FACE) {
            return "Face"
        }
        return "Idle"
    }
}

@MainActor
final class WarGameModel: ObservableObject {
    @Published var commandMode: CommandMode = .select
    @Published var selectedArmy: ArmyFilter = .all
    @Published var boardWidth: Int = 1024
    @Published var boardHeight: Int = 768
    @Published var units: [UnitSnapshot] = []
    @Published var unitTypes: [UnitTypeSnapshot] = []
    @Published var selectedCommandText: String = "No command yet."
    @Published var isPaused: Bool = false

    private var statusTimer: Cancellable?
    private var boardRect: [Int] = [0, 0, 0, 0]

    init() {
        refreshBoardDimensions()
        applyArmyFilter()
        refreshState()

        statusTimer = Timer.publish(every: 0.25, on: .main, in: .common)
            .autoconnect()
            .sink { [weak self] _ in
                self?.refreshState()
            }
    }

    func refreshBoardDimensions() {
        war_dimensions(&boardRect)
        let nextWidth = Int(boardRect[1])
        let nextHeight = Int(boardRect[2])
        boardWidth = nextWidth > 0 ? nextWidth : 1024
        boardHeight = nextHeight > 0 ? nextHeight : 768
    }

    func applyArmyFilter() {
        war_set_selected_army(selectedArmy.armyCode)
        selectedCommandText = selectedArmy == .all ? "Army filter: all" : "Army filter: \(selectedArmy.title)"
        refreshState()
    }

    func boardPoint(from point: CGPoint, in size: CGSize, flipY: Bool = true) -> CGPoint {
        let width = max(1.0, size.width)
        let height = max(1.0, size.height)

        let x = (point.x / width) * CGFloat(boardWidth)
        let sourceY = flipY ? (height - point.y) : point.y
        let y = (sourceY / height) * CGFloat(boardHeight)

        let clampedX = min(max(0.0, x), CGFloat(max(0, boardWidth - 1)))
        let clampedY = min(max(0.0, y), CGFloat(max(0, boardHeight - 1)))
        return CGPoint(x: clampedX, y: clampedY)
    }

    func issueMove(to point: CGPoint, canvasSize: CGSize) {
        let boardPoint = boardPoint(from: point, in: canvasSize)
        let destinationX = Int(boardPoint.x.rounded())
        let destinationY = Int(boardPoint.y.rounded())
        let affected = Int(war_order_move(destinationX, destinationY))

        if affected > 0 {
            selectedCommandText = "Move order sent to \(affected) unit group(s)."
        } else {
            selectedCommandText = "Move order: no selected groups for current filter."
        }
        refreshState()
    }

    func issueFace(to point: CGPoint, canvasSize: CGSize) {
        let boardPoint = boardPoint(from: point, in: canvasSize)
        let destinationX = Int(boardPoint.x.rounded())
        let destinationY = Int(boardPoint.y.rounded())
        let affected = Int(war_order_face(destinationX, destinationY))

        if affected > 0 {
            selectedCommandText = "Facing order sent to \(affected) unit group(s)."
        } else {
            selectedCommandText = "Face order: no selected groups for current filter."
        }
        refreshState()
    }

    func refreshState() {
        let unitCount = Int(war_unit_count())
        let typeCount = Int(war_unit_type_count())

        var unitTypeSnapshots: [UnitTypeSnapshot] = []
        unitTypeSnapshots.reserveCapacity(typeCount)
        if typeCount > 0 {
            for typeIndex in 0..<typeCount {
                unitTypeSnapshots.append(
                    UnitTypeSnapshot(
                        id: typeIndex,
                        name: "Type \(typeIndex)",
                        defense: Int(war_unit_type_stat(typeIndex, WarUnitTypeStatCode.defence.rawValue)),
                        meleeAttack: Int(war_unit_type_stat(typeIndex, WarUnitTypeStatCode.meleeAttack.rawValue)),
                        meleeDamage: Int(war_unit_type_stat(typeIndex, WarUnitTypeStatCode.meleeDamage.rawValue)),
                        missileAttack: Int(war_unit_type_stat(typeIndex, WarUnitTypeStatCode.missileAttack.rawValue)),
                        missileDamage: Int(war_unit_type_stat(typeIndex, WarUnitTypeStatCode.missileDamage.rawValue)),
                        speed: Int(war_unit_type_stat(typeIndex, WarUnitTypeStatCode.speedMaximum.rawValue)),
                        stature: Int(war_unit_type_stat(typeIndex, WarUnitTypeStatCode.stature.rawValue)),
                        woundsPerCombatant: Int(war_unit_type_stat(typeIndex, WarUnitTypeStatCode.woundsPerCombatant.rawValue)),
                        missileRange: Int(war_unit_type_stat(typeIndex, WarUnitTypeStatCode.missileRange.rawValue))
                    )
                )
            }
        }
        unitTypes = unitTypeSnapshots

        var unitSnapshots: [UnitSnapshot] = []
        unitSnapshots.reserveCapacity(unitCount)
        if unitCount > 0 {
            for index in 0..<unitCount {
                unitSnapshots.append(
                    UnitSnapshot(
                        id: index,
                        typeId: Int(war_unit_type_for_unit(index)),
                        alignment: Int(war_unit_alignment(index)),
                        alive: Int(war_unit_living(index)),
                        total: Int(war_unit_total(index)),
                        averageX: Int(war_unit_average_x(index)),
                        averageY: Int(war_unit_average_y(index)),
                        selected: Int(war_unit_selected(index)) == 1,
                        commandMode: Int(war_unit_command_mode(index))
                    )
                )
            }
        }
        units = unitSnapshots

        let selected = unitSnapshots.filter(\.selected).count
        if selected > 0 {
            selectedCommandText = "\(selected) group(s) currently selected."
        } else {
            selectedCommandText = "No group(s) selected."
        }
    }

    func handleSelectionComplete() {
        refreshState()
    }

    func togglePause() {
        war_keyReceived(112, 0)
        isPaused.toggle()
    }

    func restartScenario() {
        war_keyReceived(110, 0)
        selectedCommandText = "Scenario restarted."
        refreshState()
    }
}

struct ContentView: View {
    @StateObject private var model = WarGameModel()

    var body: some View {
        HStack(alignment: .top, spacing: 12) {
            controlColumn
            VStack(alignment: .leading, spacing: 8) {
                commandHint
                CoreGraphicsViewRepresentable(
                    commandMode: $model.commandMode,
                    boardWidth: model.boardWidth,
                    boardHeight: model.boardHeight,
                    onMove: { point, canvasSize in
                        model.issueMove(to: point, canvasSize: canvasSize)
                    },
                    onFace: { point, canvasSize in
                        model.issueFace(to: point, canvasSize: canvasSize)
                    },
                    onSelectionComplete: {
                        model.handleSelectionComplete()
                    }
                )
                .frame(width: CGFloat(model.boardWidth), height: CGFloat(model.boardHeight))
                .background(Color.black)
                .border(Color.black.opacity(0.7), width: 1)
                .cornerRadius(4)
            }
        }
        .padding(10)
    }

    private var controlColumn: some View {
        VStack(alignment: .leading, spacing: 10) {
            controls
            Divider()
            statusPanel
            Divider()
            unitList
            Spacer(minLength: 0)
        }
        .frame(width: 320)
    }

    private var commandHint: some View {
        Text(model.commandMode.hint)
            .font(.caption)
            .foregroundStyle(.yellow)
            .padding(8)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(.black.opacity(0.65))
            .cornerRadius(6)
    }

    private var controls: some View {
        VStack(alignment: .leading, spacing: 10) {
            Text("Strategic Controls")
                .font(.title2.bold())

            Picker("Army", selection: $model.selectedArmy) {
                ForEach(ArmyFilter.allCases) { army in
                    Text(army.title).tag(army)
                }
            }
            .pickerStyle(.segmented)
            .onChange(of: model.selectedArmy) {
                model.applyArmyFilter()
            }

            Picker("Command", selection: $model.commandMode) {
                ForEach(CommandMode.allCases) { mode in
                    Text(mode.title).tag(mode)
                }
            }
            .pickerStyle(.segmented)

            HStack {
                Button(model.isPaused ? "Resume" : "Pause") {
                    model.togglePause()
                }
                Button("Restart") {
                    model.restartScenario()
                }
            }

            Text("Board: \(model.boardWidth) × \(model.boardHeight)")
                .font(.caption2)
                .foregroundStyle(.secondary)

            Text(model.selectedCommandText)
                .font(.caption)
                .foregroundStyle(.secondary)
        }
    }

    private var statusPanel: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Unit Types")
                .font(.headline)

            ScrollView {
                VStack(alignment: .leading, spacing: 6) {
                    ForEach(model.unitTypes) { type in
                        VStack(alignment: .leading, spacing: 2) {
                            Text(type.name).font(.caption2).bold()
                            Text("Defense \(type.defense), Melee \(type.meleeAttack)/\(type.meleeDamage), Range \(type.missileAttack)/\(type.missileDamage)")
                                .font(.caption2)
                            Text("Speed \(type.speed), Stature \(type.stature), Wounds \(type.woundsPerCombatant), Missile range \(type.missileRange)")
                                .font(.caption2)
                        }
                        .padding(6)
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .background(Color.black.opacity(0.05))
                        .cornerRadius(4)
                    }
                }
            }
            .frame(maxHeight: 230)
        }
    }

    private var unitList: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Unit Groups")
                .font(.headline)

            ScrollView {
                VStack(alignment: .leading, spacing: 4) {
                    ForEach(model.units) { unit in
                        let selectionTag = unit.selected ? "[SEL] " : ""
                        Text(
                            "\(selectionTag)Group \(unit.id) | Army \(unit.alignment) | Type \(unit.typeId) | " +
                            "Pos (\(unit.averageX), \(unit.averageY)) | \(unit.alive)/\(unit.total) | \(unit.commandLabel)"
                        )
                        .font(.caption2)
                        .foregroundStyle(unit.selected ? .green : .primary)
                        .background(unit.selected ? Color.green.opacity(0.12) : Color.clear)
                        .clipShape(RoundedRectangle(cornerRadius: 3))
                    }
                }
            }
            .frame(maxHeight: 220)
        }
    }
}

#Preview {
    ContentView()
}
