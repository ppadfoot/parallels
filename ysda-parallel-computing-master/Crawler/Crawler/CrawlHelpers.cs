using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;

namespace Crawler
{
    public class Link
    {
        public Link(bool isSecure, string url)
        {
            IsSecure = isSecure;
            Url = url;
        }

        public override string ToString()
        {
            return (IsSecure ? "https" : "http") + "://" + Url;
        }

        public bool IsSecure { get; }
        public string Url { get; }
    }

    public static class CrawlHelpers
    {
        public static bool TryGetPage(Link address, out string page)
        {
            using (var client = new WebClient())
            {
                client.Encoding = Encoding.UTF8;

                try
                {
                    page = client.DownloadString(address.ToString());
                    return true;
                }
                catch (WebException e)
                {
                    Console.WriteLine(e);
                    page = null;
                    return false;
                }
                catch (ArgumentException e)
                {
                    Console.WriteLine(e);
                    page = null;
                    return false;
                }
            }
        }

        public static Link[] GetLinks(string page)
        {
            return new Regex(@"<a .*?href=(""|')(.+?)(""|').*?>")
                .Matches(page)
                .Cast<Match>()
                .Select(x => x.Groups[2].Value)
                .Where(x => x.StartsWith("http://") || x.StartsWith("https://") || x.StartsWith("//"))
                .Select(StripUrl)
                .ToArray();
        }

        public static Link StripUrl(string url)
        {
            var urlWithPrefix = url.Replace("https://www.", "https://")
                .Replace("http://www.", "http://")
                .Trim('/');

            if (urlWithPrefix.StartsWith("//"))
                urlWithPrefix = "http:" + urlWithPrefix;

            return new Link(isSecure: urlWithPrefix.StartsWith("https://"), url: urlWithPrefix.Replace("https://", "").Replace("http://", ""));
        }

        public static string GetPageFilename(string address)
        {
            var pageFilename = address.Length < 100 ? address : address.Substring(0, 100);
            foreach (var invalidChar in Path.GetInvalidFileNameChars())
                pageFilename = pageFilename.Replace(invalidChar, '_');
            return pageFilename;
        }
    }
}