import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DumpShadowrunFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        java.io.PrintWriter file = null;
        String[] scriptArgs = getScriptArgs();
        if (scriptArgs.length > 0) {
            file = new java.io.PrintWriter(scriptArgs[0]);
        }
        long[] offsets = {
            0x42b0L, 0x59172L, 0x59248L, 0x59318L, 0x597eeL,
            0x10d30L, 0x10f50L, 0x110bcL, 0x11136L, 0x11190L,
            0x11442L, 0x11592L, 0x11624L, 0x116c4L, 0x1170cL,
            0x11928L, 0x11ae4L, 0x12148L, 0x12168L, 0x12c5aL,
            0x1378L, 0x3d80L, 0x5fdcL, 0x600eL, 0xaff2L
            ,0x10918L, 0x1232L, 0x3602L, 0x3648L, 0x34fcL,
            0x36fcL, 0x3404L, 0x12c36L, 0x12a8cL, 0x12e9eL,
            0x125faL, 0x111d0L, 0x11740L, 0x11506L, 0x12c42L,
            0x12c56L, 0x127b0L, 0x13000L, 0x1304eL, 0x12e00L,
            0x12d00L, 0x12f00L, 0x12f64L
        };
        for (long offset : offsets) {
            Address address = toAddr(offset);
            Function function = currentProgram.getFunctionManager().getFunctionAt(address);
            String heading = "===== " + address + " =====";
            println(heading);
            if (file != null) file.println(heading);
            if (function == null) {
                disassemble(address);
                function = createFunction(address, null);
            }
            if (function == null) {
                println("NO FUNCTION");
                if (file != null) file.println("NO FUNCTION");
                continue;
            }
            String signature = function.getSignature().getPrototypeString();
            println(signature);
            if (file != null) file.println(signature);
            DecompileResults result = decompiler.decompileFunction(function, 60, monitor);
            if (result.decompileCompleted() && result.getDecompiledFunction() != null) {
                String output = result.getDecompiledFunction().getC();
                println(output);
                if (file != null) file.println(output);
            } else {
                String error = "DECOMPILE FAILED: " + result.getErrorMessage();
                println(error);
                if (file != null) file.println(error);
            }
        }
        if (file != null) file.close();
    }
}
