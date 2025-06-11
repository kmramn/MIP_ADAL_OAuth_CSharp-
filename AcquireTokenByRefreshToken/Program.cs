using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.IdentityModel.Clients.ActiveDirectory;
using System.Globalization;
//using Microsoft.IdentityModel.Clients.ActiveDirectory;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Runtime.InteropServices;
using System.Configuration;
using System.Net;

namespace AcquireTokenByRefreshToken
{
    class Program
    {
        private static AuthenticationContext authContext = null;
        //private static string szAuthority = "https://login.microsoftonline.com/halocore.onmicrosoft.com/oauth2/authorize";
        private static string szAuthority = "https://login.windows.net/common/oauth2/authorize";

        //private static string szResource = "https://graph.windows.net/"; // This does not works
        private static string szResource = "https://api.aadrm.com/";

        private static string szClientId = "3e6c46a1-bb03-4f75-b89f-36834fa21caf"; //NativeClient-DotNet This works
        //private static string szClientId = "6096f760-a7e8-453b-9a44-52d63a2533bc"; // TodoListDaemonNoKey This does not works.
        //private static string szClientId = "6b069eef-9dde-4a29-b402-8ce866edc897"; // From MIP sample This is also working.
        //private static string szClientId = "5e1538ea-c9b0-40c9-bfac-e08903efabed"; // TodoListDaemon This does not works.

        private static string szUserName = "itadmins@halocore.onmicrosoft.com";
        private static string szPassword = "ITSEC!2345";
        //private static string szRefreshToken = "";
        private static string szRefreshToken = "";
        static void Main(string[] args)
        {
            AuthenticationResult result = null;
            authContext = new AuthenticationContext(szAuthority);
            result = authContext.AcquireTokenByRefreshToken(szRefreshToken, szClientId);
            Console.WriteLine("Access Token:" + result.AccessToken);
            Console.WriteLine("Refresh Token:" + result.RefreshToken);
        }
    }
}
