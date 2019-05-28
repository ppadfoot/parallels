using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using MoreLinq;

namespace Crawler
{
    public class PartialResult
    {
        public PartialResult()
        {
            FailedDownloads = 0;
            Links = new List<Link>();
        }

        public PartialResult WithFailedDownload()
        {
            FailedDownloads++;
            return this;
        }

        public int FailedDownloads { get; set; }
        public List<Link> Links { get; set; }
    }

    public class WebCrawlerAsync : ICrawler
    {
        public WebCrawlerAsync(string baseUrl, int maxDepth, int maxPages, string folder, int degreeOfParallelism = -1)
        {
            this.baseUrl = baseUrl;
            this.maxDepth = maxDepth;
            this.maxPages = maxPages;
            this.folder = folder;
            this.degreeOfParallelism = degreeOfParallelism;
        }

        public void Crawl(bool verbose = false)
        {
            var links = new List<Link> {CrawlHelpers.StripUrl(baseUrl)};
            var visited = new HashSet<string>(links.Select(x => x.Url));
            var currentDepth = 0;
            var downloadedPages = 0;
            while (links.Count != 0)
            {
                if (currentDepth > maxDepth)
                    break;

                var newLinks = new List<Link>();
                var failedDownloads = 0;
                var depth = currentDepth;
                var pages = downloadedPages;

                Parallel.ForEach(links, new ParallelOptions {MaxDegreeOfParallelism = degreeOfParallelism},
                    () => new PartialResult(),
                    (link, _, partialResult) =>
                    {
                        if (verbose)
                            Console.WriteLine($"Depth: {depth}/{maxDepth}, " +
                                              $"Downloaded: {pages}/{maxPages}, " +
                                              $"Downloading: {link}.");

                        if (!CrawlHelpers.TryGetPage(link, out var page))
                            return partialResult.WithFailedDownload();

                        File.WriteAllText($"{folder}/{CrawlHelpers.GetPageFilename(link.Url)}.html", page);

                        partialResult.Links.AddRange(CrawlHelpers.GetLinks(page));
                        return partialResult;
                    },
                    partialResult =>
                    {
                        newLinks.AddRange(partialResult.Links);
                        failedDownloads += partialResult.FailedDownloads;
                    });

                downloadedPages += links.Count - failedDownloads;

                links = newLinks
                    .Where(x => !visited.Contains(x.Url))
                    .DistinctBy(x => x.Url)
                    .Take(maxPages - downloadedPages)
                    .ToList();

                visited.UnionWith(links.Select(x => x.Url));
                currentDepth++;
            }
        }

        private readonly string baseUrl;
        private readonly int maxDepth;
        private readonly int maxPages;
        private readonly string folder;
        private readonly int degreeOfParallelism;
    }
}