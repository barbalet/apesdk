import SwiftUI

// This file belongs only to bronze-mac; shared shell call sites are #if BRONZE_SIM.
@MainActor
final class BronzeScenarioController: ObservableObject {
    static let names = ["first_slice", "vertical_slice", "bronze_age_full"]
    private let bundle: Bundle
    private let storage = bronze_scenario_create()
    @Published private(set) var activeName = "No scenario loaded"
    @Published private(set) var diagnostics = ""
    @Published private(set) var lastAttemptFailed = false
    @Published var showDiagnostics = false
    var seed: UInt { UInt(bronze_scenario_seed(storage)) }

    init(bundle: Bundle = .main) { self.bundle = bundle; _ = load("first_slice") }
    deinit { bronze_scenario_destroy(storage) }

    @discardableResult
    func load(_ name: String, onCommit: ((UInt) -> Void)? = nil) -> Bool {
        let filename = name + ".bronze"
        let path = bundle.url(forResource: name, withExtension: "bronze")?.path
        let accepted: Bool = filename.withCString { label in
            guard let path else {
                return bronze_scenario_load(storage, nil, label) != 0
            }
            return path.withCString { bronze_scenario_load(storage, $0, label) != 0 }
        }
        diagnostics = String(cString: bronze_scenario_diagnostics(storage))
        if accepted {
            activeName = String(cString: bronze_scenario_name(storage))
            showDiagnostics = false
            onCommit?(seed)
        }
        lastAttemptFailed = !accepted
        if !accepted { showDiagnostics = true }
        return accepted
    }
}

struct BronzeScenarioCommands: Commands {
    @ObservedObject var controller: BronzeScenarioController
    let select: (String) -> Void
    var body: some Commands {
        CommandMenu("Bronze Scenario") {
            ForEach(BronzeScenarioController.names, id: \.self) { name in
                Button("Load \(name).bronze — Reset World") { select(name) }
            }
            Divider()
            Button("Scenario Details and Diagnostics…") { controller.showDiagnostics = true }
        }
    }
}

struct BronzeScenarioPanel: View {
    @ObservedObject var controller: BronzeScenarioController
    var actorID: UInt64 = 0
    var body: some View {
        HStack {
            VStack(alignment: .leading) {
                Text(controller.activeName).font(.caption.bold())
                Text(controller.lastAttemptFailed ? "Load failed; active scenario unchanged." :
                     "Validated content; profession tasks are not active yet.")
                    .font(.caption)
                Text(actorID == 0 ? "No registered being selected" : "Actor #\(actorID) · profession unassigned")
                    .font(.caption).accessibilityIdentifier("BronzeActorIdentity")
            }
            Spacer()
            Button("Details") { controller.showDiagnostics = true }
        }
        .padding(8)
        .background(.regularMaterial)
        .accessibilityIdentifier("BronzeScenarioStatus")
        .sheet(isPresented: $controller.showDiagnostics) {
            VStack(alignment: .leading, spacing: 12) {
                Text("Active: \(controller.activeName)").font(.headline)
                ScrollView {
                    Text(controller.diagnostics).font(.system(.body, design: .monospaced))
                        .textSelection(.enabled).frame(maxWidth: .infinity, alignment: .leading)
                }
                Button("Close") { controller.showDiagnostics = false }
            }
            .padding().frame(minWidth: 520, minHeight: 320)
            .accessibilityIdentifier("BronzeScenarioDiagnostics")
        }
    }
}
