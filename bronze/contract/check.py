#!/usr/bin/env python3
"""Cycle 01 baseline and boundary checks. --record explicitly replaces snapshots."""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import platform
import plistlib
import re
import shlex
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
CC = shlex.split(os.environ.get('CC', 'cc'))


def run(args, cwd=None):
    result = subprocess.run([str(x) for x in args], cwd=cwd, text=True, capture_output=True)
    if result.returncode:
        raise RuntimeError(f"Command failed: {shlex.join(map(str, args))}\n{result.stdout[-4000:]}\n{result.stderr[-8000:]}")
    return result.stdout


def build(output, sources, flags=()):
    run(CC + ['-std=c99', '-O1', '-DCOMMAND_LINE_EXPLICIT'] + list(flags) +
        list(sources) + ['-lz', '-lm', '-lpthread', '-o', output])


def snapshot(path, actual, record):
    if record:
        path.write_text(json.dumps(actual, indent=2, sort_keys=True) + '\n')
    elif not path.exists() or json.loads(path.read_text()) != actual:
        raise RuntimeError(f'{path.name} differs: review the change before explicitly recording a new baseline.')


def audit():
    project = ROOT / 'toolchains/bronze-mac/bronze-mac.xcodeproj/project.pbxproj'
    objects = plistlib.loads(project.read_bytes())['objects']
    configs = [v['buildSettings'] for v in objects.values()
               if v.get('isa') == 'XCBuildConfiguration' and
               v['buildSettings'].get('PRODUCT_BUNDLE_IDENTIFIER') == 'com.apesdk.bronze-mac']
    assert len(configs) == 2
    for settings in configs:
        definitions = settings.get('GCC_PREPROCESSOR_DEFINITIONS', [])
        assert 'BRONZE_SIM=1' in definitions and 'HIDE_THUNDER=1' in definitions
        assert settings['CODE_SIGNING_ALLOWED'] == 'NO'
        assert 'BRONZE_SIM' in settings['SWIFT_ACTIVE_COMPILATION_CONDITIONS']
    for path in ROOT.rglob('project.pbxproj'):
        if path != project:
            assert not re.search(r'\b(?:BRONZE_SIM|HIDE_THUNDER)\b', path.read_text()), path
    # Compile the public header without relying on platform-specific transitive includes.
    run(CC + ['-std=c99', '-x', 'c', '-fsyntax-only', '-DCOMMAND_LINE_EXPLICIT',
              '-DBRONZE_SIM=1', ROOT / 'shared.h'])
    # This checks the compiler's actual result rather than trusting indentation.
    for name in ['universe/sim.c', 'universe/loop.c', 'universe/command.c', 'gui/draw.c', 'gui/shared.c', 'shared.h']:
        ordinary = run(CC + ['-E', '-P', '-DCOMMAND_LINE_EXPLICIT', ROOT / name])
        bronze = run(CC + ['-E', '-P', '-DCOMMAND_LINE_EXPLICIT', '-DBRONZE_SIM=1', ROOT / name])
        assert not re.search(r'\b(?:bronze_\w+|shared_session_bronze_(?:actor_id|land_policy|professions|actor_summary|program))\b', ordinary), f'Bronze symbols leaked into {name}'
        assert re.search(r'\b(?:bronze_(?:hybrid|actors)_\w+|shared_session_bronze_(?:actor_id|land_policy|professions|actor_summary|program))\b', bronze), f'Bronze hook absent in {name}'
    # One implementation of compiler support and native adapters, not forwarded C copies.
    for stem in ['brz_parser', 'brz_dsl', 'brz_kinds', 'brz_vec', 'brz_util']:
        assert not list((ROOT / 'bronze/runtime').glob(stem + '.*')), stem
    assert not (ROOT / 'bronze/bronze_apesdk_adapter.c').exists()
    assert not (ROOT / 'bronze/bronze_apesdk_compiled.c').exists()
    target = next(v for v in objects.values() if v.get('isa') == 'PBXNativeTarget' and v.get('name') == 'bronze-mac')
    sources = next(objects[x] for x in target['buildPhases'] if objects[x]['isa'] == 'PBXSourcesBuildPhase')
    paths = [objects[objects[x]['fileRef']]['path'] for x in sources['files']]
    for name in ['dsl/brz_parser.c', 'dsl/brz_dsl.c', 'dsl/brz_kinds.c', 'dsl/brz_vec.c', 'dsl/brz_util.c',
                 'adapter/bronze_apesdk_adapter.c', 'adapter/bronze_apesdk_compiled.c',
                 'scenario/bronze_scenario.c', 'ui/BronzeScenarioUI.swift', 'actors/bronze_actors.c', 'work/bronze_work.c', 'tasks/bronze_tasks.c']:
        assert paths.count('../../bronze/' + name) == 1, name
    resources = next(objects[x] for x in target['buildPhases'] if objects[x]['isa'] == 'PBXResourcesBuildPhase')
    bundled = [objects[objects[x]['fileRef']]['path'] for x in resources['files']]
    for name in ['example_large', 'first_slice', 'vertical_slice', 'bronze_age_full']:
        assert bundled.count('../../bronze/content/' + name + '.bronze') == 1
    print('PASS build definitions, hook isolation, canonical ownership and bundled resource references', flush=True)


def inventory(work, record):
    results = {}
    content = sorted((ROOT / 'bronze/content').glob('*.bronze'))
    for dialect in ['runtime', 'dsl']:
        folder = ROOT / 'bronze/dsl'
        binary = work / f'inventory-{dialect}'
        flags = ['-I' + str(folder)] + (['-DLEGACY_RUNTIME_PROFILE=1'] if dialect == 'runtime' else [])
        build(binary, [folder / ('brz_' + f + '.c') for f in ['parser','dsl','kinds','vec','util']] +
              [HERE / 'inventory.c'], flags)
        for path in content:
            result = subprocess.run([str(binary), str(path)], text=True, capture_output=True, check=True)
            lines = result.stdout.splitlines()
            rows = [line.split('\t') for line in lines]
            results.setdefault(path.name, {'sha256': hashlib.sha256(path.read_bytes()).hexdigest()})
            results[path.name][dialect] = {
                'accepted': rows[0] == ['accepted', '1'],
                'diagnostics': result.stderr.splitlines(),
                'counts': dict(sorted(Counter(r[0] for r in rows).items())),
                # Preserve every parsed construct, argument, condition and parameter.
                'ast_sha256': hashlib.sha256(('\n'.join(lines) + '\n').encode()).hexdigest(),
                'constructs': sorted({tuple(r) for r in rows if r[0] not in ['vocation', 'task']}),
                'vocations': [r[1] for r in rows if r[0] == 'vocation'],
            }
    # JSON normalizes tuples; compare the same representation when checking.
    results = json.loads(json.dumps(results))
    snapshot(HERE / 'content_inventory.json', results, record)
    print('PASS parsed inventory of every construct in all bundled scenarios, through both explicit profiles of the canonical compiler', flush=True)


def compiler_and_cli(work):
    folder = ROOT / 'bronze/dsl'
    compiler_sources = [folder / ('brz_' + f + '.c') for f in ['parser','dsl','kinds','vec','util']]
    unit = work / 'compiler-test'
    build(unit, compiler_sources + [HERE / 'compiler_test.c'], ['-I' + str(folder)])
    run([unit, ROOT / 'bronze/content/first_slice.bronze', ROOT / 'bronze/content/vertical_slice.bronze'], cwd=work)
    cli = work / 'bronze-sim'
    runtime = ROOT / 'bronze/runtime'
    build(cli, compiler_sources + sorted(runtime.glob('*.c')), ['-I' + str(folder), '-I' + str(runtime)])
    expected = json.loads((HERE / 'cli_baselines.json').read_text())
    for name, wanted in expected.items():
        result = subprocess.run([str(cli), str(ROOT / 'bronze/content' / name)], cwd=work, capture_output=True, check=True)
        actual = {key + '_sha256': hashlib.sha256(getattr(result, key)).hexdigest() for key in ['stdout', 'stderr']}
        assert actual == wanted, f'Standalone behavior changed for {name}'
    scenario = work / 'scenario-test'
    build(scenario, compiler_sources + [ROOT / 'bronze/scenario/bronze_scenario.c', ROOT / 'bronze/scenario/test_scenario.c'],
          ['-I' + str(folder), '-I' + str(ROOT / 'bronze/scenario')])
    run([scenario, *[ROOT / 'bronze/content' / name for name in ['first_slice.bronze', 'vertical_slice.bronze', 'bronze_age_full.bronze']]], cwd=work)
    print('PASS compiler/CLI snapshots and transactional scenario load/bounds/diagnostic tests', flush=True)


def baselines(work, record):
    sources = [p for folder in ['toolkit','script','render','sim','entity','universe']
               for p in sorted((ROOT / folder).glob('*.c'))]
    traces = {}
    for mode, flags in [('native', []), ('native_hide', ['-DHIDE_THUNDER=1']),
                        ('bronze', ['-DBRONZE_SIM=1', '-DHIDE_THUNDER=1'])]:
        binary = work / mode
        extra = []
        if mode == 'bronze':
            extra = [ROOT / 'bronze/hybrid/bronze_hybrid.c', ROOT / 'bronze/actors/bronze_actors.c', ROOT / 'bronze/work/bronze_work.c', ROOT / 'bronze/tasks/bronze_tasks.c'] + [ROOT / 'gui' / (f + '.c') for f in ['draw','buffer','tutorial']]
        build(binary, sources + extra + [HERE / 'baseline.c'], flags)
        traces[mode] = []
        for seed in [42, 1337]:
            def trace():
                return [line for line in run([binary, seed], cwd=work).splitlines() if line.startswith('BASE ')]
            first, second = trace(), trace()
            assert first and first == second, f'Nonrepeatable {mode}, seed {seed}'
            traces[mode].extend(first)
        # Ordinary binary is linked without any bronze/*.c sources.
        if mode != 'bronze':
            symbols = run(['nm', binary])
            assert not re.search(r'\b_?bronze_\w+', symbols), f'Bronze dependency in {mode}'
        print(f'PASS {mode}: seeds 42/1337, cycles 1/60/120, repeated fresh-process traces', flush=True)
    def native_rows(mode):
        return [line for line in traces[mode] if line.startswith('BASE native ')]
    assert native_rows('native_hide') == native_rows('bronze'), 'Bronze prototype changed sampled native state'
    assert native_rows('native') == native_rows('native_hide'), 'HIDE_THUNDER changed sampled non-lightning state'
    key = f'{platform.system()}-{platform.machine()}'
    path = HERE / 'baselines.json'
    stored = json.loads(path.read_text()) if path.exists() else {}
    if record:
        stored[key] = traces
        snapshot(path, stored, True)
    elif key in stored:
        assert stored[key] == traces, 'Recorded baseline changed; investigate before re-recording'
    else:
        print(f'NOTE no pinned baseline for {key}; repeatability/differential checks passed, record on a reviewed host', flush=True)
    print('PASS native observable equivalence across all three modes; layout included; original links independently', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--record', action='store_true')
    args = parser.parse_args()
    audit()
    with tempfile.TemporaryDirectory(prefix='bronze-contract-') as tmp:
        inventory(Path(tmp), args.record)
        compiler_and_cli(Path(tmp))
        baselines(Path(tmp), args.record)
