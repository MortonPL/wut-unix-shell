%option nounput
%option noinput

/*definitions*/

%% /*rules*/
[0-9]   return(1);
.       return(0);

%% /*user actions*/
