// RUN: iree-opt -split-input-file -iree-vm-sink-defining-ops %s | IreeFileCheck %s

vm.module @module {
  // CHECK-LABEL: @single_uses
  vm.func @single_uses(%arg0 : i32) -> i32 {
    %c4 = vm.const.i32 4 : i32
    %c5 = vm.const.i32 5 : i32
    // CHECK: vm.cond_br
    vm.cond_br %arg0, ^bb1, ^bb2
  ^bb1:
    //      CHECK: %c4 = vm.const.i32 4 : i32
    // CHECK-NEXT: vm.return %c4 : i32
    vm.return %c4 : i32
  ^bb2:
    //      CHECK: %c5 = vm.const.i32 5 : i32
    // CHECK-NEXT: vm.return %c5 : i32
    vm.return %c5 : i32
  }
}

// -----

vm.module @module {
  // CHECK-LABEL: @multiple_uses
  vm.func @multiple_uses(%arg0 : i32) -> (i32, i32) {
    %c4 = vm.const.i32 4 : i32
    %c5 = vm.const.i32 5 : i32
    // CHECK: %c5 = vm.const.i32 5 : i32
    // CHECK: vm.cond_br
    vm.cond_br %arg0, ^bb1, ^bb2
  ^bb1:
    //      CHECK: %c4 = vm.const.i32 4 : i32
    // CHECK-NEXT: vm.return %c4, %c5
    vm.return %c4, %c5 : i32, i32
  ^bb2:
    // CHECK: vm.return %c5 : i32
    vm.return %c5 : i32
  }
}

// -----

vm.module @module {
  // CHECK-LABEL: @common_dominator
  vm.func @common_dominator(%arg0 : i32, %arg1 : i32) -> (i32, i32) {
    %c4 = vm.const.i32 4 : i32
    %c5 = vm.const.i32 5 : i32
    %c6 = vm.const.i32 6 : i32
    //      CHECK: %c5 = vm.const.i32 5 : i32
    // CHECK-NEXT: vm.cond_br %arg0
    vm.cond_br %arg0, ^bb1, ^bb_end
  ^bb1:
    //      CHECK: "test.do_something_else"
    "test.do_something_else"() : () -> ()
    // CHECK-NEXT: %c4 = vm.const.i32 4 : i32
    // CHECK-NEXT: "test.do_thing"(%c4) : (i32) -> ()
    "test.do_thing"(%c4) : (i32) -> ()
    // CHECK-NEXT: vm.cond_br %arg1
    vm.cond_br %arg1, ^bb2, ^bb_end
  ^bb2:
    "test.do_thing"(%c4) : (i32) -> ()
    "test.do_thing"(%c5) : (i32) -> ()
    // CHECK: vm.br
    vm.br ^bb_end
  ^bb_end:
    //      CHECK: %c6 = vm.const.i32 6 : i32
    // CHECK-NEXT: vm.return %c5, %c6
    vm.return %c5, %c6 : i32, i32
  }
}
