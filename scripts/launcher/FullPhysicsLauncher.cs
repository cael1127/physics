using System;
using System.Diagnostics;
using System.IO;

internal static class FullPhysicsLauncher
{
    private static int RunProcess(string fileName, string arguments, string workingDir)
    {
        var psi = new ProcessStartInfo
        {
            FileName = fileName,
            Arguments = arguments,
            WorkingDirectory = workingDir,
            UseShellExecute = false,
            RedirectStandardOutput = false,
            RedirectStandardError = false,
            CreateNoWindow = false
        };

        using (var p = Process.Start(psi))
        {
            if (p == null)
            {
                Console.Error.WriteLine("Failed to start process: " + fileName);
                return 1;
            }
            p.WaitForExit();
            return p.ExitCode;
        }
    }

    private static string Quote(string s)
    {
        return "\"" + s + "\"";
    }

    public static int Main(string[] args)
    {
        try
        {
            var exeDir = AppDomain.CurrentDomain.BaseDirectory;
            var repoRoot = exeDir.TrimEnd('\\');
            var installScript = Path.Combine(repoRoot, "scripts", "install_prerequisites.ps1");
            var runScript = Path.Combine(repoRoot, "scripts", "run_app.ps1");

            Console.WriteLine("=== FullPhysicsLauncher.exe ===");
            Console.WriteLine("Repo: " + repoRoot);

            if (!File.Exists(installScript))
            {
                Console.Error.WriteLine("Missing script: " + installScript);
                Console.WriteLine("Press Enter to exit...");
                Console.ReadLine();
                return 1;
            }
            if (!File.Exists(runScript))
            {
                Console.Error.WriteLine("Missing script: " + runScript);
                Console.WriteLine("Press Enter to exit...");
                Console.ReadLine();
                return 1;
            }

            bool runOnly = false;
            bool clean = false;
            bool skipInstall = false;
            foreach (var a in args)
            {
                if (string.Equals(a, "--run-only", StringComparison.OrdinalIgnoreCase)) runOnly = true;
                if (string.Equals(a, "--clean", StringComparison.OrdinalIgnoreCase)) clean = true;
                if (string.Equals(a, "--skip-install", StringComparison.OrdinalIgnoreCase)) skipInstall = true;
            }

            if (!skipInstall)
            {
                Console.WriteLine();
                Console.WriteLine("[1/2] Ensuring prerequisites...");
                var installArgs = "-NoProfile -ExecutionPolicy Bypass -File " + Quote(installScript);
                var code = RunProcess("powershell.exe", installArgs, repoRoot);
                if (code != 0)
                {
                    Console.Error.WriteLine("Prerequisite step failed (exit " + code + ").");
                    Console.WriteLine("Press Enter to exit...");
                    Console.ReadLine();
                    return code;
                }
            }

            Console.WriteLine();
            Console.WriteLine("[2/2] Building and launching FullPhysicsC...");
            var runArgs = "-NoProfile -ExecutionPolicy Bypass -File " + Quote(runScript);
            if (runOnly) runArgs += " -RunOnly";
            if (clean) runArgs += " -Clean";

            var runCode = RunProcess("powershell.exe", runArgs, repoRoot);
            if (runCode != 0)
            {
                Console.Error.WriteLine("Build/launch failed (exit " + runCode + ").");
                Console.WriteLine("Press Enter to exit...");
                Console.ReadLine();
                return runCode;
            }

            Console.WriteLine("Launcher completed successfully.");
            return 0;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex.ToString());
            Console.WriteLine("Press Enter to exit...");
            Console.ReadLine();
            return 1;
        }
    }
}
