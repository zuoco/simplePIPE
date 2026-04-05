// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

export module pipecad.runtime.document;

export namespace app {
class Document;
class SelectionManager;
}

export namespace pipecad::runtime {
using Document = ::app::Document;
using SelectionManager = ::app::SelectionManager;
}