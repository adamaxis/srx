// Headless export for the one canonical Shadowrun Genesis Ghidra project.
// Run with analyzeHeadless -postScript ExportShadowrun.java <output-dir>.

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.listing.Program;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceManager;
import ghidra.program.model.symbol.RefType;
import ghidra.util.task.TaskMonitor;

import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import ghidra.program.model.symbol.SourceType;

public class ExportShadowrun extends GhidraScript {
    // Reviewed names are deliberately kept here, beside the export logic, so
    // every generated artifact uses the same vocabulary.  An address without
    // enough evidence remains Ghidra's FUN_<address> name until it is
    // understood and recorded in the scratchpad.
    private static Map<Long, String> reviewedNames() {
        Map<Long, String> names = new LinkedHashMap<>();
        names.put(0x00000236L, "reset_and_enter_boot");
        names.put(0x00000436L, "write_startup_text_to_vdp");
        names.put(0x0000073AL, "wait_for_vdp_ready");
        names.put(0x0000075EL, "run_game_loop");
        names.put(0x00000BD2L, "wait_for_vblank");
        names.put(0x00000BF2L, "latch_controller_events");
        names.put(0x00000C2CL, "clear_runtime_memory_and_vdp");
        names.put(0x00000D44L, "advance_source_prng");
        names.put(0x00001088L, "step_palette_toward_black");
        names.put(0x000010D6L, "step_palette_toward_target");
        names.put(0x0000131CL, "step_palette_transition");
        names.put(0x00001342L, "wait_for_palette_frame");
        names.put(0x00001368L, "copy_target_palette");
        names.put(0x00001358L, "copy_source_word_block");
        names.put(0x000014CEL, "extract_even_bytes");
        names.put(0x00001502L, "copy_even_bytes");
        names.put(0x00001610L, "decode_source_byte_stream");
        names.put(0x0000167CL, "decode_source_coordinate_stream");
        names.put(0x00003E86L, "expand_packed_nibbles");
        names.put(0x000041B6L, "fold_field_coordinate");
        names.put(0x00004216L, "offset_field_coordinate");
        names.put(0x0000423CL, "quantize_field_delta");
        names.put(0x00004E38L, "advance_entity_primary_timer");
        names.put(0x00004E64L, "advance_entity_secondary_timer");
        names.put(0x00005A56L, "project_entity_screen_position");
        names.put(0x00005A62L, "project_entity_screen_position_standard");
        names.put(0x00005B1EL, "sort_entity_slots_by_depth");
        names.put(0x00005B7EL, "mark_entity_slots_for_render");
        names.put(0x00005FDCL, "advance_entity_animation_accumulator");
        names.put(0x000061A0L, "append_entity_sprite_record");
        names.put(0x000061CCL, "append_sprite_record");
        names.put(0x0000622CL, "expand_entity_sprite_descriptors");
        names.put(0x0000624CL, "expand_entity_sprite_descriptor_list");
        names.put(0x0000B6E4L, "select_entity_animation_table");
        names.put(0x0000B48CL, "evaluate_entity_collision");
        names.put(0x0000B442L, "reset_entity_movement_state");
        names.put(0x0000B39AL, "set_entity_heading_with_field_animation");
        names.put(0x0000B3FEL, "set_entity_heading_with_alternate_animation");
        names.put(0x0000AF6CL, "reset_entity_animation_state");
        names.put(0x0000ABFCL, "update_entity_animation_delay");
        names.put(0x0000D7EAL, "compute_entity_delay_factor");
        names.put(0x0000D81CL, "average_entity_delay_attributes");
        names.put(0x0000AF7CL, "prepare_entity_delayed_animation_primary");
        names.put(0x0000AFAEL, "prepare_entity_delayed_animation_alternate");
        names.put(0x0000AFF2L, "prepare_entity_auxiliary_behavior");
        names.put(0x0000B330L, "enter_entity_behavior_transition");
        names.put(0x0000B11CL, "synchronize_entity_behavior_spawn");
        names.put(0x0000B1DCL, "configure_entity_target_transition");
        names.put(0x0000B49CL, "resolve_entity_fallback_movement");
        names.put(0x0000BB86L, "evaluate_entity_movement_route");
        names.put(0x0000BADEL, "advance_entity_movement_step");
        names.put(0x0000E8A0L, "sample_terrain_collision_tiles");
        names.put(0x0000E9EEL, "contains_terrain_object_bounds");
        names.put(0x0000EAD2L, "evaluate_terrain_direction_boundary");
        names.put(0x0000EDFAL, "decode_terrain_tile_window");
        names.put(0x0000EE10L, "rebuild_terrain_collision_map");
        names.put(0x0000EE86L, "replace_nth_terrain_marker");
        names.put(0x0000FEBEL, "move_selection_column");
        names.put(0x0000FF06L, "move_selection_row");
        names.put(0x0000C376L, "move_wrapped_menu_cursor");
        names.put(0x0000C8B0L, "apply_byte_counter_delta_with_negative_floor");
        names.put(0x00015CA4L, "compute_diagonal_step_cost");
        names.put(0x00015CC0L, "compute_diagonal_step_cost");
        names.put(0x0000B732L, "apply_movement_result_policy");
        names.put(0x0000B77EL, "probe_entity_directional_collision");
        names.put(0x0000B530L, "approach_entity_fixed_point_target");
        names.put(0x0000BBE4L, "commit_entity_position");
        names.put(0x0000BC36L, "build_collision_probe_points");
        names.put(0x0000BCAEL, "apply_mode_five_boundary_motion");
        names.put(0x0000C134L, "normalize_source_text_line");
        names.put(0x0000195CL, "sample_controller_input");
        names.put(0x000019F2L, "read_controller_port");
        names.put(0x00001BACL, "finalize_title_input_sequence");
        names.put(0x00001BEAL, "record_title_input_edge");
        names.put(0x00001C1EL, "advance_title_text_state");
        names.put(0x00001C6AL, "advance_title_sprite_state");
        names.put(0x00001D7AL, "setup_title_state");
        names.put(0x00001DCAL, "draw_title_prompt_variant_a");
        names.put(0x00001DE0L, "draw_title_prompt_variant_b");
        names.put(0x00001DF6L, "draw_title_copyright");
        names.put(0x00001E4EL, "reveal_title_logo_column");
        names.put(0x00002634L, "cycle_title_circuit_lights");
        names.put(0x000026D0L, "advance_title_animation_timer");
        names.put(0x00001A9EL, "run_title_loop");
        names.put(0x00003084L, "run_field_loop");
        names.put(0x00005D746L, "run_pre_title_sequence");
        names.put(0x00005DA50L, "setup_pre_title_resources");
        names.put(0x00005DAC4L, "fill_boot_horizontal_scroll");
        names.put(0x00005D8BAL, "step_pre_title_logo_strobe");
        names.put(0x00005D938L, "advance_pre_title_animation");
        names.put(0x00005D996L, "advance_pre_title_logo_phase");
        return names;
    }

    private static void applyReviewedNames(Program program) throws Exception {
        for (Map.Entry<Long, String> entry : reviewedNames().entrySet()) {
            Address address = program.getAddressFactory().getDefaultAddressSpace()
                .getAddress(entry.getKey());
            Function function = program.getFunctionManager().getFunctionAt(address);
            if (function != null && !entry.getValue().equals(function.getName())) {
                function.setName(entry.getValue(), SourceType.USER_DEFINED);
            }
        }
    }

    private static String hex(Address address) {
        return String.format("%08X", address.getOffset());
    }

    private static String clean(String value) {
        return value.replace('\t', ' ').replace('\r', ' ').replace('\n', ' ');
    }

    private static String fileName(Function function) {
        return String.format("fn_%08X.cpp", function.getEntryPoint().getOffset());
    }

    private static List<Function> calledFunctions(Function function,
                                                   TaskMonitor monitor) throws Exception {
        Set<Function> called = new HashSet<>();
        for (Address address : function.getBody().getAddresses(true)) {
            monitor.checkCanceled();
            Instruction instruction = function.getProgram().getListing()
                .getInstructionAt(address);
            if (instruction == null || !instruction.getFlowType().isCall()) {
                continue;
            }
            for (Address target : instruction.getFlows()) {
                Function callee = function.getProgram().getFunctionManager()
                    .getFunctionAt(target);
                if (callee != null) {
                    called.add(callee);
                }
            }
        }
        List<Function> result = new ArrayList<>(called);
        result.sort(Comparator.comparing(Function::getEntryPoint));
        return result;
    }

    private static String callAnnotation(Function caller, Instruction instruction,
                                         Program program) {
        if (!instruction.getFlowType().isCall()) {
            return "";
        }
        StringBuilder result = new StringBuilder();
        for (Address target : instruction.getFlows()) {
            Function callee = program.getFunctionManager().getFunctionAt(target);
            if (result.length() != 0) {
                result.append(", ");
            }
            result.append("call ").append(hex(target));
            if (callee != null) {
                result.append(" ").append(clean(callee.getName()));
            } else {
                result.append(" [unresolved]");
            }
        }
        return result.toString();
    }

    private static void writeFunction(Program program, Function function,
                                      List<Function> calls, DecompInterface decompiler,
                                      Path output, TaskMonitor monitor) throws IOException {
        DecompileResults result = decompiler.decompileFunction(function, 120, monitor);
        String body;
        String status;
        if (result.decompileCompleted() && result.getDecompiledFunction() != null) {
            body = result.getDecompiledFunction().getC();
            status = "ok";
        } else {
            body = "/* decompilation failed: " + clean(result.getErrorMessage()) + " */\n";
            status = "failed";
        }

        Path file = output.resolve("ghidra-c").resolve(fileName(function));
        try (BufferedWriter writer = Files.newBufferedWriter(file,
                StandardCharsets.UTF_8)) {
            writer.write("// Ghidra headless export\n");
            writer.write("// Entry: 0x" + hex(function.getEntryPoint()) + "\n");
            writer.write("// Name: " + clean(function.getName()) + "\n");
            writer.write("// Decompile status: " + status + "\n");
            writer.write("// Calls recovered from direct call instructions:\n");
            if (calls.isEmpty()) {
                writer.write("//   (none)\n");
            } else {
                for (Function callee : calls) {
                    writer.write("//   0x" + hex(callee.getEntryPoint()) + " " +
                                 clean(callee.getName()) + "\n");
                }
            }
            writer.write("\n");
            writer.write(body);
            writer.write("\n");
        }
    }

    @Override
    public void run() throws Exception {
        String[] scriptArguments = getScriptArgs();
        if (scriptArguments.length == 0 || scriptArguments[0].trim().isEmpty()) {
            throw new IllegalArgumentException("ExportShadowrun requires an output directory");
        }

        Path output = Paths.get(scriptArguments[0]).toAbsolutePath().normalize();
        Files.createDirectories(output.resolve("ghidra-c"));
        Program program = currentProgram;
        applyReviewedNames(program);
        FunctionManager functions = program.getFunctionManager();
        List<Function> entries = new ArrayList<>();
        for (Function function : functions.getFunctions(true)) {
            entries.add(function);
        }
        entries.sort(Comparator.comparing(Function::getEntryPoint));

        DecompInterface decompiler = new DecompInterface();
        decompiler.setSimplificationStyle("decompile");
        if (!decompiler.openProgram(program)) {
            throw new IOException("could not open program in decompiler");
        }

        try (BufferedWriter functionTable = Files.newBufferedWriter(
                     output.resolve("functions.tsv"), StandardCharsets.UTF_8);
             BufferedWriter callTable = Files.newBufferedWriter(
                     output.resolve("callgraph.tsv"), StandardCharsets.UTF_8);
             BufferedWriter callSiteTable = Files.newBufferedWriter(
                     output.resolve("call-sites.tsv"), StandardCharsets.UTF_8);
             BufferedWriter disassembly = Files.newBufferedWriter(
                     output.resolve("disassembly.s"), StandardCharsets.UTF_8);
             BufferedWriter report = Files.newBufferedWriter(
                     output.resolve("program-report.txt"), StandardCharsets.UTF_8)) {
            functionTable.write("entry\tname\tsize\tcallingConvention\tdecompileStatus\tfile\n");
            callTable.write("caller\tcaller_name\tcallee\tcallee_name\tcall_sites\n");
            callSiteTable.write("caller\tcaller_name\tinstruction\ttext\ttarget\tcallee\tstatus\n");
            report.write("Program: " + program.getName() + "\n");
            report.write("Language: " + program.getLanguageID() + "\n");
            report.write("Compiler: " + program.getCompilerSpec().getCompilerSpecID() + "\n");
            report.write("Image base: " + program.getImageBase() + "\n");
            report.write("Max address: " + program.getMaxAddress() + "\n");
            report.write("Processor: " + program.getLanguage().getProcessor() + "\n");
            report.write("Function count: " + entries.size() + "\n");

            for (Function function : entries) {
                monitor.checkCanceled();
                List<Function> calls = calledFunctions(function, monitor);
                writeFunction(program, function, calls, decompiler, output, monitor);
                long size = function.getBody().getNumAddresses();
                String status = "unknown";
                DecompileResults check = decompiler.decompileFunction(function, 1, monitor);
                if (check.decompileCompleted()) {
                    status = "ok";
                }
                functionTable.write(hex(function.getEntryPoint()) + "\t" +
                                   clean(function.getName()) + "\t" + size + "\t" +
                                   clean(function.getCallingConventionName()) + "\t" +
                                   status + "\tghidra-c/" + fileName(function) + "\n");
                for (Function callee : calls) {
                    int sites = 0;
                    for (Address address : function.getBody().getAddresses(true)) {
                        Instruction instruction = program.getListing().getInstructionAt(address);
                        if (instruction != null && instruction.getFlowType().isCall()) {
                            for (Address target : instruction.getFlows()) {
                                if (target.equals(callee.getEntryPoint())) {
                                    ++sites;
                                }
                            }
                        }
                    }
                    callTable.write(hex(function.getEntryPoint()) + "\t" +
                                    clean(function.getName()) + "\t" +
                                    hex(callee.getEntryPoint()) + "\t" +
                                    clean(callee.getName()) + "\t" + sites + "\n");
                }
                for (Address address : function.getBody().getAddresses(true)) {
                    Instruction instruction = program.getListing().getInstructionAt(address);
                    if (instruction == null || !instruction.getFlowType().isCall()) {
                        continue;
                    }
                    Address[] targets = instruction.getFlows();
                    if (targets.length == 0) {
                        callSiteTable.write(hex(function.getEntryPoint()) + "\t" +
                                            clean(function.getName()) + "\t" +
                                            hex(address) + "\t" +
                                            clean(instruction.toString()) +
                                            "\t\t\tindirect/unresolved\n");
                        continue;
                    }
                    for (Address target : targets) {
                        Function callee = program.getFunctionManager().getFunctionAt(target);
                        callSiteTable.write(hex(function.getEntryPoint()) + "\t" +
                                            clean(function.getName()) + "\t" +
                                            hex(address) + "\t" +
                                            clean(instruction.toString()) + "\t" +
                                            hex(target) + "\t" +
                                            (callee == null ? "" : clean(callee.getName())) +
                                            "\t" +
                                            (callee == null ? "unresolved" : "resolved") +
                                            "\n");
                    }
                }
            }

            Listing listing = program.getListing();
            ReferenceManager references = program.getReferenceManager();
            for (Instruction instruction : listing.getInstructions(true)) {
                monitor.checkCanceled();
                String annotation = callAnnotation(
                    functions.getFunctionContaining(instruction.getAddress()),
                    instruction, program);
                disassembly.write(hex(instruction.getAddress()) + ": " +
                                  clean(instruction.toString()));
                if (!annotation.isEmpty()) {
                    disassembly.write(" ; " + annotation);
                } else {
                    Reference[] refs = references.getReferencesFrom(instruction.getAddress());
                    if (refs.length != 0 && refs[0].getReferenceType() == RefType.DATA) {
                        disassembly.write(" ; data " + refs[0].getToAddress());
                    }
                }
                disassembly.write("\n");
            }
        } finally {
            decompiler.dispose();
        }
    }
}
