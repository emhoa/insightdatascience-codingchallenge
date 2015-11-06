#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

extern int errno;

// timestamp associated with each hashtag    
struct timestamp_st {
    int dayofmonth;
    int dayofweek;
    int month;
    int yr;
    int hr;
    int min;
    int sec;
};
    
// linked list of hashtags connected with one particular hashtag
struct connected_hash {
    struct hashnode *hash;
    struct timestamp_st *latesttimestamp;
    struct connected_hash *next;
};

// holds hashtag text and timestamp
struct hashnode {
    wchar_t text[144];
    struct timestamp_st latesttimestamp;
};

// linked list of hashtag and associated hashtags that make up hashgraph
struct hashgraph {
    struct hashnode *hash;
    struct connected_hash *associated_hashes;
    struct hashgraph *next;
};

// global variable holding all hashgraph
struct hashgraph *global_hashgraph;


int main() {

    FILE *rfp, *wfp;
    struct stat st;
    wint_t wc;
    int nestlevel;
    wchar_t filteredtextarray[144];
    int unicode_count=0;
    wchar_t timestampstr[40];
    int numfieldsfilled=0;
    
    filteredtextarray[0]=L'\0';
    timestampstr[0]=L'\0';

    // Open file for reading 
    if ((rfp = fopen("./tweet_input/tweets.txt", "r")) == NULL) {
        printf("Unable to open file");
        return;
    }
    
    // Open file for writing; check for directory first
    if ((stat("./tweet_output", &st)) != 0) {
        if (errno == ENOENT) {
            if ((mkdir("tweet_output", S_IRWXU)) != 0) {
                printf("Unable to create tweet_output dir");
                return;
            }
        } else {
            printf("Error in creating tweet_output dir");
            return;
        }
    }
    if ((wfp = fopen("./tweet_output/ft1.txt", "w")) == NULL) {
        printf("Unable to open file for writing");
        return;
    }
    
    // Loop through each line of the tweets.txt
    // instead of bringing entire json into memory, just look for the two fields, tweets and timestamps
    nestlevel=0;
    while ((wc = fgetwc(rfp)) != WEOF) {
        
        int i;

        //tweet text and timestamp handling
        if (nestlevel == 1 && wc == L'"') 
        {
            // tweet text
            if ((wc=fgetwc(rfp)) == L't') 
            {
                if ((wc=fgetwc(rfp)) == L'e' &&
                    (wc=fgetwc(rfp)) == L'x' &&
                    (wc=fgetwc(rfp)) == L't' &&
                    (wc=fgetwc(rfp)) == L'"' &&
                    (wc=fgetwc(rfp)) == L':' &&
                    (wc=fgetwc(rfp)) == L'"')
                {
                    // found the tweets section
                    
                    wchar_t textarray[1000];
                    int i;
                    wint_t lastwc;
            
             
                    // fill up rest of the tweet text; search for ending quote but make sure it's not part of escaped sequence
                    for (i=0, lastwc=L'\0'; (wc=fgetwc(rfp)) != L'"' || (wc == L'"' && lastwc == L'\\'); i++) {    
                    
                         textarray[i] = wc;
                         lastwc = wc;
                    } 
                    textarray[i]=L'\0';
                    if (filteroutunicode(textarray, filteredtextarray)>0)
                        unicode_count++;
                    
                    numfieldsfilled++;
                }
            } else if (wc == L'c' && 
                (wc=fgetwc(rfp)) == L'r' &&
                (wc=fgetwc(rfp)) == L'e' &&
                (wc=fgetwc(rfp)) == L'a' &&
                (wc=fgetwc(rfp)) == L't' &&
                (wc=fgetwc(rfp)) == L'e' &&
                (wc=fgetwc(rfp)) == L'd' &&
                (wc=fgetwc(rfp)) == L'_' &&
                (wc=fgetwc(rfp)) == L'a' &&
                (wc=fgetwc(rfp)) == L't' &&
                (wc=fgetwc(rfp)) == L'"' &&
                (wc=fgetwc(rfp)) == L':' &&
                (wc=fgetwc(rfp)) == L'"')  {
            
                //timestamp handling    
                int i;
                for (i=0; (wc=fgetwc(rfp)) != L'"'; i++) {    
                    timestampstr[i]=wc;                    
                }
                timestampstr[i]=L'\0';
                numfieldsfilled++;
            }
        }
        
        
      if (numfieldsfilled == 2) {
        
            // first feature: print into file
            fputws(filteredtextarray, wfp);
            fputws(L" (timestamp: ", wfp);
            fputws(timestampstr, wfp);
            fputwc(L')', wfp);
            fputwc(L'\n', wfp);

            // (incomplete) second feature: find hashtags if any, and add to global hashgraph (function, addhashgraphifany, commented out because it crashes on some examples)
            // addhashgraphifany(filteredtextarray, timestampstr);
                        
            // trim global_hashgraph

            // recalc rolling average
            
            // print to file
            

            //got what we needed; fast forward to next line
            while ((wc=fgetwc(rfp)) != L'\n') {
                if (wc == WEOF) {
                    printunicodecount_closefps(unicode_count, wfp, rfp);
                    return;
                };
            }
            nestlevel=0;
            
            //reset timestamp and filteredtextarray strings
            timestampstr[0]=L'\0';
            filteredtextarray[0]=L'\0';
            numfieldsfilled=0;

        }
        
        if (wc == L'{')
             nestlevel++;
        else if (wc == L'}')
             nestlevel--;
        else if (wc == WEOF)
            break;
    }
    
    printunicodecount_closefps(unicode_count, wfp, rfp);
}

// function main runs to gracefully exit and close up file pointers
int printunicodecount_closefps(int unicode_count, FILE *wfp, FILE *rfp) {
    wchar_t *unicode_countstr;
    struct hashgraph *tmphashgraphptr;
    struct connected_hash *tmpassociatedhashptr, *myptr;
    
    unicode_countstr=malloc(sizeof(wchar_t)*unicode_count);
    swprintf(unicode_countstr, unicode_count, L"%d", unicode_count);
    fputwc(L'\n', wfp);
    fputws(unicode_countstr, wfp);
    fputws(L" tweets contained unicode", wfp);
    
    fclose(wfp);
    fclose(rfp);
    free(unicode_countstr);
    
    printglobalhashgraph();
    tmphashgraphptr = global_hashgraph;
    while (tmphashgraphptr != NULL) {
        global_hashgraph=tmphashgraphptr->next;
        tmpassociatedhashptr = tmphashgraphptr->associated_hashes;
        while (tmpassociatedhashptr != NULL) {
            myptr = tmpassociatedhashptr;
            tmpassociatedhashptr = tmpassociatedhashptr->next;
            free(myptr);
        }
        free(tmphashgraphptr->hash);
        free(tmphashgraphptr);
        tmphashgraphptr = global_hashgraph;
    }
}

// main function to filter out unicode from input string and send to output string
int filteroutunicode(wchar_t *inputstring, wchar_t *outputstring) {
 
 int i, j;
 wint_t wc;
 int unicode_count = 0;
 
 for (i=0, j=0, wc=inputstring[i]; wc != L'\0'; wc = inputstring[i]) {
    if (wc == L'\\') {
        if ((wc=inputstring[i+1]) == L'u' ) {
            unicode_count++;    
            i=i+6;
        }
        else if (wc == L'/')
            i++;
        else if (wc == L'\"' || wc == L'\'' || wc == L'\\') {
            outputstring[j]=wc;
            i=i+2;
            j++;
        }
        else {
            i=i+2;

        }
    } else if (wc == L'&' && inputstring[i+1] == L'a' && inputstring[i+2] == L'm' && inputstring[i+3] == L'p' && inputstring[i+4] == L';') {
            outputstring[j] = wc;
            j++;
            i=i+5;
    }
    else {
        outputstring[j] = inputstring[i];
        i++;
        j++;
    }
    
 }
 outputstring[j]=L'\0';
 return unicode_count;
}

// convert timestamp string to object
int convert_timestamp(wchar_t *timestampstr, struct timestamp_st *timestampobj) {
    
    wchar_t dow[5];
    wchar_t mon[5];
    wchar_t ignoreme[5];        

    timestampobj->dayofweek=0;
    timestampobj->month=0;
    timestampobj->dayofmonth=0;
    timestampobj->hr=0;
    timestampobj->min=0;
    timestampobj->sec=0;
    
    swscanf(timestampstr, L"%ls %ls %d %d:%d:%d %ls %d", &dow, &mon, &(timestampobj->dayofmonth), &(timestampobj->hr), &(timestampobj->min), &(timestampobj->sec), &ignoreme, &(timestampobj->yr));
    dow[5]=L'\0';
    mon[5]=L'\0';

        
    if ((wcscmp(dow,L"Mon"))==0) timestampobj->dayofweek = 1;
    else if ((wcscmp(dow,L"Tue"))==0) timestampobj->dayofweek = 2;
    else if ((wcscmp(dow,L"Wed"))==0) timestampobj->dayofweek = 3;
    else if ((wcscmp(dow,L"Thu"))==0) timestampobj->dayofweek = 4;
    else if ((wcscmp(dow,L"Fri"))==0) timestampobj->dayofweek = 5;
    else if ((wcscmp(dow,L"Sat"))==0) timestampobj->dayofweek = 6;
    else if ((wcscmp(dow,L"Sun"))==0) timestampobj->dayofweek = 7;
    
    if ((wcscmp(mon,L"Jan"))==0) timestampobj->month = 1;
    else if ((wcscmp(mon,L"Feb"))==0) timestampobj->month = 2;
    else if ((wcscmp(mon,L"Mar"))==0) timestampobj->month = 3;
    else if ((wcscmp(mon,L"Apr"))==0) timestampobj->month = 4;
    else if ((wcscmp(mon,L"May"))==0) timestampobj->month = 5;
    else if ((wcscmp(mon,L"Jun"))==0) timestampobj->month = 6;
    else if ((wcscmp(mon,L"Jul"))==0) timestampobj->month = 7;
    else if ((wcscmp(mon,L"Aug"))==0) timestampobj->month = 8;
    else if ((wcscmp(mon,L"Sep"))==0) timestampobj->month = 9;
    else if ((wcscmp(mon,L"Oct"))==0) timestampobj->month = 10;
    else if ((wcscmp(mon,L"Nov"))==0) timestampobj->month = 11;
    else if ((wcscmp(mon,L"Dec"))==0) timestampobj->month = 12;

}

// check to see timestamps differ by more than a minute; passing in strings as arguments
// returns 1 if timestampstr2 > timestampstr1 by more than minute; 0 if less than minute; -1 if timestampstr1 is newer
int diffmorethanmin(wchar_t *timestampstr1, wchar_t *timestampstr2) {
    wchar_t dow1[4];
    wchar_t mon1[4];
    int dom1;
    int hr1;
    int min1;
    int sec1;
    int yr1;
    wchar_t ignoreme1[5];    

    wchar_t dow2[4];
    wchar_t mon2[4];
    int dom2;
    int hr2;
    int min2;
    int sec2;
    int yr2;
    wchar_t ignoreme2[5];
    int mindiff;
    int secdiff;

    swscanf(timestampstr1, L"%ls %ls %d %d:%d:%d %ls %d", &dow1, &mon1, &dom1, &hr1, &min1, &sec1, &ignoreme1, &yr1);
    mon1[4]=L'\0';
    dow1[4]=L'\0';
    swscanf(timestampstr2, L"%ls %ls %d %d:%d:%d %ls %d", &dow2, &mon2, &dom2, &hr2, &min2, &sec2, &ignoreme2, &yr2);
    mon2[4]=L'\0';
    dow2[4]=L'\0';
    

     if (yr1 == yr2 && dom1==dom2 && (wcscmp(mon1,mon2))==0 && hr1 == hr2) {
        mindiff = min2 - min1;
        if (mindiff > 1 || mindiff < -1) {
            // difference is definitely more than 1 minute
            return 1;
        } else if (mindiff == 1 || mindiff == -1) {
           if ((secdiff = (((min2*60)+sec2) - ((min1*60)+sec1))) > 60 || secdiff < -60) {
               // more than 1 minute
               return 1;
           } else return 0;
        } else if (mindiff == 0) {
            // difference is definitely less than 1 minute; it's seconds
            return 0;
        }
    }
    
    return 1;
}

// check to see timestamps differ by more than a minute; passing in timestampobjects as arguments rather than strings
int timestampdiffmorethanmin(struct timestamp_st *compare1, struct timestamp_st *compare2) {
    int mindiff=0, secdiff=0;
    if (compare1->yr == compare2->yr && compare1->month == compare2->month && compare1->dayofmonth == compare2->dayofmonth && compare1->hr == compare2->hr) {
        mindiff = compare1->min - compare2->min;
        if (mindiff>1 || mindiff<-1) {
            // difference is definitely more than 1 minute
            return 1;
        } else if (mindiff == 1 || mindiff == -1) {
            if ((secdiff = (((compare2->min*60)+compare2->sec) - ((compare1->min*60)+compare1->sec))) > 60 || secdiff <-60) {
                // more than 1 minute
                return 1;
            } else return 0;
        } else if (mindiff == 0) {
            // difference is definitely less than 1 minute; it's seconds
            return 0;
        }
    }
    return 1;
}

// function to build hashgraph (not working)
int addhashgraphifany(wchar_t *tweet, wchar_t *timestampstr) {
    wchar_t *strptr, local_hashstr[144];
    int tweetlen=wcslen(tweet);
    int count_to_hash=0, count_to_space=0;
    struct hashgraph *tmp_hashgraph=NULL, *globalhashgraphptr=NULL;
    struct hashnode *local_hashnode=NULL;
    struct hashnode *local_hashnodelist[80];
    struct connected_hash *tmp_associated_hash=NULL, *local_associated_hashes=NULL;
    int numhashnodes=0;
    int i, j, k, foundassociatedmatch=0, foundglobalmatch=0, numglobaladds=0, founddup=0;
            
    if ((count_to_hash=wcscspn(tweet, L"#")) == tweetlen) {
        // no hashtags found
        
        return 0;
    } else if (count_to_hash<tweetlen) {
        
        // at least one hashtag found
        strptr=tweet+count_to_hash;
        if (strptr[1] != L' ' && strptr[1] !=L'#') {
            swscanf(strptr, L"%ls", &local_hashstr);
            count_to_space = wcscspn(strptr, L" ");
            local_hashstr[count_to_space]=L'\0';
        
            createhashnode(local_hashstr, timestampstr, &local_hashnodelist[numhashnodes]);
            numhashnodes++;
        } else strptr=strptr+2;
        
        //look for more hashes
        for (strptr = strptr+count_to_space; (count_to_hash = wcscspn(strptr, L"#")) < wcslen(strptr); strptr=strptr+count_to_space) {
            
            strptr=strptr+count_to_hash;
            if (strptr[1] != L' ' && strptr[1] != L'#') {
                swscanf(strptr, L"%ls", &local_hashstr);
                count_to_space = wcscspn(strptr, L" ");
                local_hashstr[count_to_space]=L'\0';
                //create hashnode only if not a duplicate
                
                for (i=0, founddup=0; i<numhashnodes; i++) {
                    if (wcscmp(local_hashnodelist[i]->text, local_hashstr) == 0) {
                        founddup=1; 
                        break;
                    }
                }
                if (founddup == 0) {
                    createhashnode(local_hashstr, timestampstr, &local_hashnodelist[numhashnodes]);
                    numhashnodes++;
                }
            } else count_to_space = 2;
        }
    } else {
        return -1;
    }
// Debugging wprintfs    
//    wprintf(L"numhashnodes: %d ", numhashnodes);
//    wprintf(L"Tweet has these hashtags/timestamps: ");
//    for (i=0; i<numhashnodes; i++) {
//        wprintf(L"%ls (%d/%d/%d %d:%d:%d)\n", local_hashnodelist[i]->text, (local_hashnodelist[i]->latesttimestamp)->month, (local_hashnodelist[i]->latesttimestamp)->dayofmonth, (local_hashnodelist[i]->latesttimestamp)->yr, (local_hashnodelist[i]->latesttimestamp)->hr, (local_hashnodelist[i]->latesttimestamp)->min, (local_hashnodelist[i]->latesttimestamp)->sec);
//   }    
    if (numhashnodes<2) {
        //done here, no need to add one hash tweets
        for (i=0; i<numhashnodes; i++) free(local_hashnodelist[i]);
        return i;
    } else {
        // add hash twets
        for (i=0; i<numhashnodes; i++) {
            // check to see if this hash is already on the global list
            for (globalhashgraphptr = global_hashgraph, foundglobalmatch=0; globalhashgraphptr != NULL; globalhashgraphptr = globalhashgraphptr->next) {
                if (wcscmp(local_hashnodelist[i]->text, globalhashgraphptr->hash->text) == 0) {
                    //hash text matches, see if associated are a match
                    for (j=0; j<numhashnodes; j++) {
                        if (j != i) {
                            for (foundassociatedmatch=0, tmp_associated_hash = globalhashgraphptr->associated_hashes; tmp_associated_hash != NULL; tmp_associated_hash = tmp_associated_hash->next) {
                              if (wcscmp(tmp_associated_hash->hash->text, local_hashnodelist[j]->text) == 0) {
                                // found node, update timestamp
                                tmp_associated_hash->latesttimestamp = &local_hashnodelist[j]->latesttimestamp;
                                foundassociatedmatch=1;
                                break;
                              }
                            }
                            if (foundassociatedmatch == 0) {
                                //create an associated node
                                tmp_associated_hash = malloc(sizeof(struct connected_hash));
                                tmp_associated_hash->hash = local_hashnodelist[j];
                                tmp_associated_hash->latesttimestamp = &local_hashnodelist[j]->latesttimestamp;
                                tmp_associated_hash->next = NULL;
                                // put it at the end of local_associated_hashes, which will then eventually go onto globalhaslgraphptr->associated_hashes
                                if (local_associated_hashes == NULL) local_associated_hashes = tmp_associated_hash;
                                else {
                                    // tack tmp_associated_hash to the front of local_associated_hashes
                                    tmp_associated_hash->next = local_associated_hashes;
                                    local_associated_hashes = tmp_associated_hash;
                                }    
                            }    
                        } //end of searching through associated_hashes to see if there is a match
                    } //end of for loop for searching through associated hashes to see if there is a match
                    // almost done. remember to add local_associated_hashes to the end of current globalhashgraph->associated_hashes
                    if (globalhashgraphptr->associated_hashes == NULL) globalhashgraphptr->associated_hashes = local_associated_hashes;
                    else {
                        local_associated_hashes->next = globalhashgraphptr->associated_hashes;
                        globalhashgraphptr->associated_hashes = local_associated_hashes;
                        }
                    foundglobalmatch=1;
                    break;
                }
            }
            if (foundglobalmatch == 0) {
                //no match on local_hashnodelist[i] so need to add to globalhashgraph
                tmp_hashgraph = malloc(sizeof(struct hashgraph));
                tmp_hashgraph->hash = local_hashnodelist[i];
                tmp_hashgraph->associated_hashes=NULL;
                tmp_hashgraph->next=NULL;
                                     
                // go through rest of connected hashes creating associated_hashes               
                for (k=0; k<numhashnodes; k++) {
                    if (k != i) {
                        tmp_associated_hash = malloc(sizeof(struct connected_hash));
                        tmp_associated_hash->hash = local_hashnodelist[k];
                        tmp_associated_hash->latesttimestamp = &local_hashnodelist[k]->latesttimestamp;
                        tmp_associated_hash->next = NULL;
                        if (tmp_hashgraph->associated_hashes == NULL) {
                            tmp_hashgraph->associated_hashes=tmp_associated_hash;
                        } else {
                            tmp_associated_hash->next = tmp_hashgraph->associated_hashes;
                            tmp_hashgraph->associated_hashes = tmp_associated_hash;
                        }
                    }
                }
                // now add tmp_hashgraph to global_hashgraph
                if (global_hashgraph == NULL) global_hashgraph = tmp_hashgraph;
                else {
                    tmp_hashgraph->next = global_hashgraph;
                    global_hashgraph = tmp_hashgraph;
                }                        
                numglobaladds++;
            }    
        } // end of going through all of our new hashnodes and checking to see if they are on global hash graph
    }
    return numglobaladds;
}

// Pointer to hashnode passed into function to malloc space and assign values
int createhashnode(wchar_t *myhashstr, wchar_t *timestampstr, struct hashnode **hashnoderesult) {
        
        struct hashnode *localhashnode;

        wint_t myhashstr_char;
        int i=0;
        
        while (myhashstr[i])  {
            myhashstr_char = myhashstr[i];
            myhashstr[i]=towlower(myhashstr_char);
            i++;
        }
        
        (*hashnoderesult) = malloc(sizeof(struct hashnode));
        wcsncpy((*hashnoderesult)->text, myhashstr, wcslen(myhashstr));
        convert_timestamp(timestampstr, &((*hashnoderesult)->latesttimestamp));
}

//debugging tool
int printglobalhashgraph() {
    struct hashgraph *globalhashgraphptr;
    struct connected_hash *tmpassociatedhashptr;

    globalhashgraphptr=global_hashgraph;
    while (globalhashgraphptr != NULL) {
        wprintf(L"Top level hashnode: %ls (%d/%d/%d %d:%d:%d)\n", globalhashgraphptr->hash->text, (globalhashgraphptr->hash->latesttimestamp).month, (globalhashgraphptr->hash->latesttimestamp).dayofmonth, (globalhashgraphptr->hash->latesttimestamp).yr, (globalhashgraphptr->hash->latesttimestamp).hr, (globalhashgraphptr->hash->latesttimestamp).min, (globalhashgraphptr->hash->latesttimestamp).sec);
        tmpassociatedhashptr = globalhashgraphptr->associated_hashes;
        while (tmpassociatedhashptr != NULL) {
            wprintf(L" - %ls (%d/%d/%d %d:%d:%d)\n", tmpassociatedhashptr->hash->text, (tmpassociatedhashptr->hash->latesttimestamp).month, (tmpassociatedhashptr->hash->latesttimestamp).dayofmonth, (tmpassociatedhashptr->hash->latesttimestamp).yr, (tmpassociatedhashptr->hash->latesttimestamp).hr, (tmpassociatedhashptr->hash->latesttimestamp).min, (tmpassociatedhashptr->hash->latesttimestamp).sec);
            tmpassociatedhashptr = tmpassociatedhashptr->next;
        }
        globalhashgraphptr=globalhashgraphptr->next;
    }
}

