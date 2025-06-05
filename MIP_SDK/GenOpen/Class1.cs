using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Identity.Client;
//using Microsoft.InformationProtection;

namespace GenOpen
{
    internal class Class1
    {
        //private ApplicationInfo _appInfo;
        // Microsoft Authentication Library IPublicClientApplication
        private IPublicClientApplication _app;
        public static string AcquireToken(string authority, string resource, string claims)
        {
            IPublicClientApplication _app;
            var authorityUri = new Uri(authority);
            // 
            //authority = String.Format("https://{0}/{1}", authorityUri.Host, "ca5f4678-4c69-4563-9130-6f9a3b0d7615");
            authority = String.Format("https://{0}/{1}", authorityUri.Host, "0f899cdd-d244-4532-94b3-6982cf130627");

            //_app = PublicClientApplicationBuilder.Create("2bdc6418-1745-4c34-8bc9-b471b909b6db")
            //                                     .WithAuthority(authority)
            //                                     .WithDefaultRedirectUri()
            //                                     .Build();
            _app = PublicClientApplicationBuilder.Create("9ed1f576-dada-42f4-aa49-877b2e71bedd")
                                                 .WithAuthority(authority)
                                                 .WithRedirectUri("https://halocadapp")
                                                 .Build();

            var accounts = (_app.GetAccountsAsync())
                           .GetAwaiter()
                           .GetResult();

            // Append .default to the resource passed in to AcquireToken().     
            string[] scopes = new string[] { resource[resource.Length - 1].Equals('/') ? $"{resource}.default" : $"{resource}/.default" };
            var result = _app.AcquireTokenInteractive(scopes)
                             .WithAccount(accounts.FirstOrDefault())
                             .WithPrompt(Prompt.SelectAccount)
                             .ExecuteAsync()
                             .ConfigureAwait(false)
                             .GetAwaiter()
                             .GetResult();
            Console.WriteLine(claims);
            Console.WriteLine(result.AccessToken);
            return result.AccessToken;
        }
        static public void Main(String[] args)
        {
            Console.WriteLine("Main Method");
            AcquireToken("https://login.windows.net/", "https://syncservice.o365syncservice.com/", "SCC");
            AcquireToken("https://login.windows.net/", "https://api.aadrm.com/", "Protection");
        }
    }
}