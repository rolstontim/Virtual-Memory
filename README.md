
# CSC 360, April, 2021
## Assignment #4 submission

* Name: `Tim, Rolston`
* VNum: `V00920780`

---

### Description of implementation of FIFO, LRU and 2ND CHANCE for assingment #4

#### Assumptions and other notes:

These implementations assume that when filling up the page table, from being empty, will 
have the pages inserted in order. That is, in terms of an array, they will be
inserted starting at index 0 and then 1 and so on...

Withing the virtmem.c file I added a few global variables; `fifo_tracker`,
`victim_frame`, and `time_lru` all of which were used to track different
values within their related functions - `fifo_tracker` for FIFO, 
`victim_frame` for 2ND CHANCE, and `time_lru` for LRU. I also added two int
variables within `page_table_entry`; `reference_bit` used for 2ND chance and
`most_frequent` used for LRU.

For 2ND CHANCE, my implementation assumes the scenario that when a page is first
added to the page table, the `reference_bit` is set to 1, that is, when a fault
occurs that recently added page will be set to 0 before getting chosen for the
victim frame. *(More in my description on 2ND CHANCE)*

#### Added resources:

As said earlier I added *3 global variables* along with *2 ints* within 
`page_table_entry`q. Along with these I also created *3 new functions*:

##### `void first_in_first_out(int memwrite, long page)`:   	 
 
This was the function I used to run the FIFO algorithm on the page table. Taking
inputs of memwrite and page from the previous function, so as to retain their 
values, this function is designed to find the page that was inserted into the
page table first, ie was inserted before all other pages. To do this I use
a global variable, `fifo_tracker`, that starts out pointing to index 0. This
will always be the **first** "oldest page". The "oldest page" is then chosen
as the victim frame and `fifo_tracker` is incremented to point to the next index,
which should be the next "oldest page". At the end of the function a check is 
done to ensure `fifo_tracker` is not greater than the variable `size_of_memory`
(size of the page table). If this is the case then `fifo_tracker` is set back
to 0 so it can continue to loop through the page table from the beginning. Within
this function a few checks are also done to ensure that if a `memwrite` is *TRUE*
then the `dirty` bit is set for the new page and if the page being swapped out
has its `dirty` bit set then we increment `swap_outs`.


##### `void least_recently_used(int memwrite, long page)`:

This is the function I used to run the LRU algorithm on the page table. It took
memwrite and page inputs from the function that calls it to be able to use their
values. This function was designed to find the least recently used page within
the page table and replace it with the new page. In order to achieve this, I
utilized a global variable, `time_lru` and a `page_table_entry` variable,
`most_frequent`. In order to find the least recently used page, I kept track
of the "time" a page was added into the table. This "time" was an int of 
`time_lru` and was stored in `page_table_entry` for each page for individual
access. This function relied on some additional lines of code added in
`resolve_address`. When the first page is inserted into an empty table, it is
given a `most_frequent` value of 0, that is `time_lru` starts at 0. After that
assignment `time_lru` is incremented by 1 and the next page is inserted. If a 
"hit" occurs then the `most_frequent` value for that page is updated and
`time_lru` is incremented. Throughout the code, anytime we update `most_frequent`
we increment `time_lru`. So in `least_recently_used` we loop through the page
table finding  the **lowest** `most_frequent` value in the table and we keep that
value. We then loop through the table once more, find the LRU page and replace
it. Then with that new page we update its `most_frequent` and increment `time_lru`.
And of course throughout the function we ensure to keep continuity of setting
and checking the `dirty` bits when required to keep a proper count of `swap_outs`.


##### `void second_chance(int memewrite, long page)`:

This is the function used to run the 2ND CHANCE algorithm on the page table,
taking both memwrite and page as inputs from the function that calls it. This
function is designed to give initialized pages a "second chance" before removing
them from the page table. I did this with the help of a global variable,
`victim_frame` and a `page_table_entry` variable, `refrence_bit`. `victim_frame`
represented where to begin searching, in the page table, for the next victim.
This value, at the beginning of the function, was always ensured to be less than
the size of the `page_table` array and set back to 0 if it ever exceeded it.
the `reference_bit` was either set to 1 or 0 to determine if a page has already
been given a "second chance" or not. This relied on the fact that when a new page
is added to the table, their `reference_bit` gets set to 1, and if a "hit" is
to occur, then that page also gets its `reference_bit` set to 1. with
`victim_frame` starting at index 0 of the page table, we loop through the table
checking if the current page's `reference_bit` is 0, if it is then we can choose
that page as the victim. Other wise if the current page's `reference_bit` is 1
then change it to 0 and increment `victim_frame`. This is done continuously,
looping back to the start of the page table if necessary, until a page with
`reference_bit` == 0 is found. It should also be noted that the next time a
page fault occurs, the search begins where the victim frame left off, so as
to follow the proper actions of the 2ND CHANCE algorithm. Throughout this function
the required testing for `dirty` bits is also done to continue the proper
addition of `swap_outs`.

#### Final notes

I just wanted to mention some added code within the `resolve_address` function.
I implemented various checks for ensuring proper functionality of the LRU and
2ND CHANCE implementations when a page is added into an empty table as well as
when a "hit" occurs. Along with these there are also proper checks for when
`memwrite` is *TRUE* and I set the `dirty` bit accordingly. Finally if a page
fault occurs and there are no empty slots then based on what the user inputs
for the `page_replacement_scheme` different functions are called to utilize
the proper algorithm to replace a page. Once the function finds a page to replace
the value is returned and the next page operation can occur within the program.
 
