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
        //private static string szRefreshToken = "AQABAAAAAADX8GCi6Js6SK82TsD2Pb7rG6D1Kr6y84dnN5oaVyX9lE4sZPPsg_S8v0QKIYSJOut3OHhf55zjnBt94H7fGzBTGmivprXVkHLvf9lbc_bpwbfkIZbHHdSoVENgS7TbV3SPOe4SXuxEW1fl3-IdeuuSXPVpT6c1v5PN2e7EVCpoysICWDWCdKTyi4bWMRpN743rnzwP8zRse3tT-hKorjo_DzZh2YwJRmtSJw_5rTRoagv_Ezct-KcZxHWWHTx3qb0HrkLMFebMw1abY6jJIPOuuBxM9cTqoWnz8SkPzXKFfTnxCBHHobB1PSY8PW7dGbLC3E7zfMx_HhyLMZcD0_wgV4BkFPUiJ5Q1zBr5_Cyd6oczYkCGnNTJEMGr8sGwBh4zwFEWMGAjTjQMaSOj6xCKvmZORlqbV6LDgNvRpxTQFSa8Fj4AQojBExJ7u7k3IjCjFPDXgJ37pR4_o_sKPffi2oNwvxeU9VHDWPUSOzIaOHwXnJ-v3zxmVmH5uNz1MqfDadfTFRq_FW1sQEl3hhqwOg-wwUX4VWUyNBYtV6eAVG8gzD5GqcCM_XirLGCAHXe2gbvNFOuD8hMlGTNzZ98cJE0EaZAheD2oQ8-GANpZglGW7wkkypCyuDxH9IMPWLwdsrkCD9aFOjIYGEZOsKd7NSL9aE31RCNAC07He4un-ToUUTkzxJMbjNq9y53lPWucumr1fNcRPa-YH8e0vc3532bNJJbdLqt7K65ZpiElfTCnFcS5Gc7F5X5ok21xhzYgAA";
        private static string szRefreshToken = "AQABAAAAAADX8GCi6Js6SK82TsD2Pb7r_rNeyu202VGTagxL7jyhGB6-gN72yLTnhEiaErXEUJ5j8OyCec60OOimJzAZMf5FidfTOjr--qxmTOG648kc4ybKz4p-UyrdBzSJsGkE-x73gQmGLFoN1et7gJXcUOWDMkyhm12N48LBggb4nHlXMilGXqk9a_rjr5qK3Vdr6yXM_uDDH6uyHeSsCUn5_wOeykG06BEzTm7--wg1wTO8f7QHZoxaZ7ohNDIQjggUxyiGh2_LTKK4xmSGGHZJfih9qCZt2oBlo14dVY6OB9cwva2rBdT4GejoKgNW5tipml2JJlzTT6f0sXPxKthSnnvEHZyQA0G9ZXJyfj4puJ6I9ixuMxjlygn7IChtqY7PRgOiE3buv7QAHpsrHNa3XZikC4VGG-MYxYUNugGo_P21hMdlKKp5-8J5P1fN0B3CsoI7oCHT-TLeCdWUyfG1ZpWUCJgu_vRsf_eGPJqTVJ1XO0Zz_8_l7bxI_9pQ7Y5UaVli7AYNl131nFBw3HUx37svT-Jgb_Ly2yDVgZCKIA03Mc0j_V5PP4x-W6aCvDSkRsJ7g4YN2Zw7VsNVjnqiBRSP5qyj1u0kFF1x09ZFufhZQiIaQHkV3br5IPtQ47p48QlEkQOa8HG0bYGFKkAiXFk3lhnGz3NtblbQcqnpdj0U3Yucq1XS-QMConfbrPa0FjlXPilEJv91HiTHBAG276CKLGvHZXUxBLZGmuBqRf1Lw7bB3GjgjQahO5TWfJMfhdUgAA";
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
