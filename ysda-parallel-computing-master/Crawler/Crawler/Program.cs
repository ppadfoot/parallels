using System;
using System.Diagnostics;
using System.IO;
using System.Linq;

namespace Crawler
{
    public class Program
    {
        private static void Measure(Action action, int n)
        {
            var measures = new double[n];
            for (var i = 0; i < n; i++)
            {
                var sw = Stopwatch.StartNew();
                action();
                sw.Stop();
                measures[i] = sw.Elapsed.TotalSeconds;
            }

            var sum = measures.Aggregate(0.0, (a, b) => a + b);
            var mean = sum / n;

            var squareSum = measures.Aggregate(0.0, (a, b) => a + b * b);
            var stdev = Math.Sqrt(squareSum / n - mean * mean);
            Console.WriteLine($"Elapsed: {mean} ({stdev})");
        }

        private static int GetArgument(string[] args, string argName)
        {
            return int.Parse(args[args.Select((x, i) => (Index: i, Arg: x)).First(x => x.Arg == argName).Index + 1]);
        }

        private static Action GetAction(string[] args)
        {
            var url = args[0];
            var maxDepth = int.Parse(args[1]);
            var maxPages = int.Parse(args[2]);
            var folder = args[3];

            var verbose = args.Contains("--verbose");
            if (args.Contains("--sync"))
                return () => new WebCrawlerSync(url, maxDepth, maxPages, folder).Crawl(verbose);

            if (!args.Contains("--threads"))
                return () => new WebCrawlerAsync(url, maxDepth, maxPages, folder).Crawl(verbose);

            var threadsCount = GetArgument(args, "--threads");
            return () => new WebCrawlerAsync(url, maxDepth, maxPages, folder, threadsCount).Crawl(verbose);
        }

        public static void Main(string[] args)
        {
            if (args.Length < 4)
                return;

            var folder = args[3];

            if (!Directory.Exists(folder))
                Directory.CreateDirectory(folder);

            var measure = args.Contains("--measure");
            var action = GetAction(args);
            if (measure)
            {
                var measurements = GetArgument(args, "--measure");
                Measure(action, measurements);
            }

            action();
        }
    }
}