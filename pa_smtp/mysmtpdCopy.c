#include "netbuffer.h"
#include "mailuser.h"
#include "server.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024

typedef enum state {
    Undefined,
    // TODO: Add additional states as necessary
    Greeting,
    Ready,
    Rcpt,
    Received,
    Quit,
} State;

typedef struct smtp_state {
    int fd;
    net_buffer_t nb;
    char recvbuf[MAX_LINE_LENGTH + 1];
    char *words[MAX_LINE_LENGTH];
    int nwords;
    State state;
    struct utsname my_uname;
    // TODO: Add additional fields as necessary
    user_list_t *users;
} smtp_state;
    
static void handle_client(int fd);

int main(int argc, char *argv[]) {
  
    if (argc != 2) {
	fprintf(stderr, "Invalid arguments. Expected: %s <port>\n", argv[0]);
	return 1;
    }
  
    run_server(argv[1], handle_client);
  
    return 0;
}

// syntax_error returns
//   -1 if the server should exit
//    1  otherwise
int syntax_error(smtp_state *ms) {
    if (send_formatted(ms->fd, "501 %s\r\n", "Syntax error in parameters or arguments") <= 0) return -1;
    return 1;
}

// checkstate returns
//   -1 if the server should exit
//    0 if the server is in the appropriate state
//    1 if the server is not in the appropriate state
int checkstate(smtp_state *ms, State s) {
    if (ms->state != s) {
	if (send_formatted(ms->fd, "503 %s\r\n", "Bad sequence of commands") <= 0) return -1;
	return 1;
    }
    return 0;
}

// All the functions that implement a single command return
//   -1 if the server should exit
//    0 if the command was successful
//    1 if the command was unsuccessful

int do_quit(smtp_state *ms) {
    dlog("Executing quit\n");
    // TODO: Implement this function
    if (send_formatted(ms->fd, "221 %s Service closing transmission channel\r\n", ms->my_uname.nodename) <= 0) return -1;
    ms->state = Quit;
    return -1;
}

int do_helo(smtp_state *ms) {
    dlog("Executing helo\n");
    // TODO: Implement this function
    ms->state = Ready;
    if(send_formatted(ms->fd, "250 %s Hello %s\r\n", ms->my_uname.nodename, ms->words[1]) <= 0){
        *ms->users = user_list_create();
        return -1;
    } 
    
    return 0;
}

int do_rset(smtp_state *ms) {
    dlog("Executing rset\n");
    // TODO: Implement this function
    ms->state = Greeting;
    
    if (send_formatted(ms->fd, "250 State reset\r\n") <= 0) {
        user_list_destroy(*ms->users);
        return -1;
    }
    return 0;
}

int do_mail(smtp_state *ms) {
    dlog("Executing mail\n");
    // TODO: Implement this function
    printf("%s", ms->words[0]);
    if (ms->state != Ready ) if(send_formatted(ms->fd, "503 Bad sequence of commands\r\n")<=0) return -1;
    if (ms->nwords < 2 || strncmp(ms->words[1], "FROM:<", 6) != 0) return syntax_error(ms);
    ms->state = Rcpt;
    if (send_formatted(ms->fd, "250 Requested mail action ok, completed\r\n") <= 0) return -1;
    return 0;
}     

int do_rcpt(smtp_state *ms) {
    dlog("Executing rcpt\n");
    // TODO: Implement this function
    if (ms->state != Rcpt ) if(send_formatted(ms->fd, "503 Bad sequence of commands\r\n")<=0) return -1;
    if (ms->nwords < 2 || strncmp(ms->words[1], "TO:<", 4) != 0) return syntax_error(ms);

    char *address = ms->words[1];
    char * trimmed = trim_angle_brackets(address+3);
    if (!is_valid_user(address+3, ms->words[2])) {
        if (send_formatted(ms->fd, "550 No such user here - %s\r\n",trimmed) <= 0) return -1;
        return 1;
    }
    ms->state = Rcpt;
    user_list_add(ms->users, trimmed);
    return 0;
}     

int do_data(smtp_state *ms) {
    dlog("Executing data\n");
    // TODO: Implement this function
    if (ms->state != Rcpt) if (send_formatted(ms->fd, "503 Bad sequence of commands\r\n") <= 0) return -1;
    if (send_formatted(ms->fd, "354 Waiting for data, finish with <CR><LF>.<CR><LF>\r\n") <= 0) return -1;

    ms->state = Ready;
    if (send_formatted(ms->fd, "250 OK: Message received\r\n") <= 0) return -1;

    return 0;
}     
      
int do_noop(smtp_state *ms) {
    dlog("Executing noop\n");
    // TODO: Implement this function
    if (send_formatted(ms->fd, "250 OK(noop)\r\n") <= 0) return -1;
    return 0;
}

int do_vrfy(smtp_state *ms) {
    dlog("Executing vrfy\n");
    // TODO: Implement this function
    char *address = ms->words[1];
    char *trimmed = trim_angle_brackets(address);
    char *pass = ms->words[2];
    
    if(address == NULL) return syntax_error(ms);
    if(is_valid_user(address, pass)) {
        if(send_formatted(ms->fd,"250 %s is a valid mailbox\r\n", trimmed) <=0) return -1;
    } else {
        if (send_formatted(ms->fd, "550 No such user here - %s\r\n", trimmed) <= 0) return -1;
    }
    
    return 0;
}

void handle_client(int fd) {
  
    size_t len;
    smtp_state mstate, *ms = &mstate;
  
    ms->fd = fd;
    ms->nb = nb_create(fd, MAX_LINE_LENGTH);
    ms->state = Undefined;
    uname(&ms->my_uname);
    
    if (send_formatted(fd, "220 %s Service ready\r\n", ms->my_uname.nodename) <= 0) return;

  
    while ((len = nb_read_line(ms->nb, ms->recvbuf)) >= 0) {
	if (ms->recvbuf[len - 1] != '\n') {
	    // command line is too long, stop immediately
	    send_formatted(fd, "500 Syntax error, command unrecognized\r\n");
	    break;
	}
	if (strlen(ms->recvbuf) < len) {
	    // received null byte somewhere in the string, stop immediately.
	    send_formatted(fd, "500 Syntax error, command unrecognized\r\n");
	    break;
	}
    
	// Remove CR, LF and other space characters from end of buffer
	while (isspace(ms->recvbuf[len - 1])) ms->recvbuf[--len] = 0;
    
	dlog("Command is %s\n", ms->recvbuf);
    
	// Split the command into its component "words"
	ms->nwords = split(ms->recvbuf, ms->words);
	char *command = ms->words[0];
    
	if (!strcasecmp(command, "QUIT")) {
	    if (do_quit(ms) == -1) break;
	} else if (!strcasecmp(command, "HELO") || !strcasecmp(command, "EHLO")) {
	    if (do_helo(ms) == -1) break;
	} else if (!strcasecmp(command, "MAIL")) {
	    if (do_mail(ms) == -1) break;
	} else if (!strcasecmp(command, "RCPT")) {
	    if (do_rcpt(ms) == -1) break;
	} else if (!strcasecmp(command, "DATA")) {
	    if (do_data(ms) == -1) break;
	} else if (!strcasecmp(command, "RSET")) {
	    if (do_rset(ms) == -1) break;
	} else if (!strcasecmp(command, "NOOP")) {
	    if (do_noop(ms) == -1) break;
	} else if (!strcasecmp(command, "VRFY")) {
	    if (do_vrfy(ms) == -1) break;
	} else if (!strcasecmp(command, "EXPN") ||
		   !strcasecmp(command, "HELP")) {
	    dlog("Command not implemented \"%s\"\n", command);
	    if (send_formatted(fd, "502 Command not implemented\r\n") <= 0) break;
	} else {
	    // invalid command
	    dlog("Illegal command \"%s\"\n", command);
	    if (send_formatted(fd, "500 Syntax error, command unrecognized\r\n") <= 0) break;
	}
    }
  
    nb_destroy(ms->nb);
}
