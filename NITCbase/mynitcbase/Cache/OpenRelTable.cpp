#include "OpenRelTable.h"
#include <iostream>
#include <cstring>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

    // initialize relCache and attrCache with nullptr
    for (int i = 0; i < MAX_OPEN; ++i) {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }

    /************ Setting up Relation Cache entries ************/
    // (we need to populate relation cache with entries for the relation catalog
    //  and attribute catalog.)

    /**** setting up Relation Catalog relation in the Relation Cache Table****/
    RecBuffer relCatBlock(RELCAT_BLOCK);

    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

    struct RelCacheEntry relCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

    // allocate this on the heap because we want it to persist outside this function
    RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

    /**** setting up Attribute Catalog relation in the Relation Cache Table ****/
    relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

    // set up the relation cache entry for the attribute catalog similarly
    // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

    // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
    RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry;

    /************ Setting up Attribute cache entries ************/
    // (we need to populate attribute cache with entries for the relation catalog
    //  and attribute catalog.)

    /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
    RecBuffer attrCatBlock(ATTRCAT_BLOCK);

    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

    // iterate through all the attributes of the relation catalog and create a linked
    // list of AttrCacheEntry (slots 0 to 5)
    // for each of the entries, set
    //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
    //    attrCacheEntry.recId.slot = i   (0 to 5)
    //    and attrCacheEntry.next appropriately
    // NOTE: allocate each entry dynamically using malloc
    struct AttrCacheEntry *head, *last;
    for(int i = 0; i < 6; i++) {
        attrCatBlock.getRecord(attrCatRecord, i);
        struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;
        if(i == 0) {
            head = attrCacheEntry;
            last = attrCacheEntry;
        }
        else {
            last->next = attrCacheEntry;
            last = attrCacheEntry;
        }
    }

    // set the next field in the last entry to nullptr
    last->next = nullptr;
    AttrCacheTable::attrCache[RELCAT_RELID] = head;

    /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

    // set up the attributes of the attribute cache similarly.
    // read slots 6-11 from attrCatBlock and initialise recId appropriately
    for(int i = 6; i < 12; i++) {
        attrCatBlock.getRecord(attrCatRecord, i);
        struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;
        if(i == 6) {
            head = attrCacheEntry;
            last = attrCacheEntry;
        }
        else {
            last->next = attrCacheEntry;
            last = attrCacheEntry;
        }
    }
    // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
    last->next = nullptr;
    AttrCacheTable::attrCache[ATTRCAT_RELID] = head;


    /************ Setting up tableMetaInfo entries ************/
    // in the tableMetaInfo array
    //   set free = false for RELCAT_RELID and ATTRCAT_RELID
    //   set relname for RELCAT_RELID and ATTRCAT_RELID
    for(int i = 0; i < MAX_OPEN; i++) {
        if(i == RELCAT_RELID) {
            tableMetaInfo[i].free = false;
            strcpy(tableMetaInfo[i].relName, RELCAT_RELNAME);
        }
        else if(i == ATTRCAT_RELID) {
            tableMetaInfo[i].free = false;
            strcpy(tableMetaInfo[i].relName, ATTRCAT_RELNAME);
        }
        else
            tableMetaInfo[i].free = true;
    }
}

OpenRelTable::~OpenRelTable() {
    // close all open relations (from rel-id = 2 onwards. Why?)
    for(int i = 2; i < MAX_OPEN; i++) {
        if(!tableMetaInfo[i].free) {
            OpenRelTable::closeRel(i);
        }
    }

    /**** Closing the catalog relations in the relation cache ****/

    //releasing the relation cache entry of the attribute catalog

    if (RelCacheTable::relCache[ATTRCAT_RELID]->dirty) {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        RelCatEntry relCatEntry = RelCacheTable::relCache[ATTRCAT_RELID]->relCatEntry;
        Attribute relCatRecord[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&relCatEntry, relCatRecord);

        RecId recId = RelCacheTable::relCache[ATTRCAT_RELID]->recId;
        // declaring an object of RecBuffer class to write back to the buffer
        RecBuffer relCatBlock(recId.block);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
        relCatBlock.setRecord(relCatRecord, recId.slot);
    }
    // free the memory dynamically allocated to this RelCacheEntry
    free(RelCacheTable::relCache[ATTRCAT_RELID]);


    //releasing the relation cache entry of the relation catalog

    if(RelCacheTable::relCache[RELCAT_RELID]->dirty) {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        RelCatEntry relCatEntry = RelCacheTable::relCache[RELCAT_RELID]->relCatEntry;
        Attribute relCatRecord[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&relCatEntry, relCatRecord);
        
        // declaring an object of RecBuffer class to write back to the buffer
        RecId recId = RelCacheTable::relCache[RELCAT_RELID]->recId;
        RecBuffer relCatBlock(recId.block);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
        relCatBlock.setRecord(relCatRecord, recId.slot);
    }
    // free the memory dynamically allocated for this RelCacheEntry
    free(RelCacheTable::relCache[RELCAT_RELID]);


    // free the memory allocated for the attribute cache entries of the
    // relation catalog and the attribute catalog
    for(int i = 0; i < 2; i++) {
        struct AttrCacheEntry* entry = AttrCacheTable::attrCache[i];
        while(entry != nullptr) {
            struct AttrCacheEntry* temp = entry;
            entry = entry->next;
            free(temp);
        }
    }
}

int OpenRelTable::getRelId(char* relName) {

    /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/
    for(int i = 0; i < MAX_OPEN; i++) {
        if(!tableMetaInfo[i].free && strcmp(tableMetaInfo[i].relName, relName) == 0)
            return i;
    }

    // if found return the relation id, else indicate that the relation do not
    // have an entry in the Open Relation Table.
    return E_RELNOTOPEN;
}

int OpenRelTable::getFreeOpenRelTableEntry() {

    /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
    for(int i = 0; i < MAX_OPEN; i++) {
        if(tableMetaInfo[i].free)
            return i;
    }

    // if found return the relation id, else return E_CACHEFULL.
    return E_CACHEFULL;
}

int OpenRelTable::openRel(char *relName) {
    int relId = getRelId(relName);
    
    if(relId >= 0) {
        return relId;
    }
    
    /* find a free slot in the Open Relation Table
        using OpenRelTable::getFreeOpenRelTableEntry(). */
    relId = getFreeOpenRelTableEntry();

    if (relId < 0){
        return E_CACHEFULL;
    }

    /****** Setting up Relation Cache entry for the relation ******/

    /* search for the entry with relation name, relName, in the Relation Catalog using
        BlockAccess::linearSearch().
        Care should be taken to reset the searchIndex of the relation RELCAT_RELID
        before calling linearSearch().*/
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute relationName;
    strcpy(relationName.sVal, relName);
    char attrRelName[] = RELCAT_ATTR_RELNAME;

    // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
    RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, attrRelName, relationName, EQ);

    if (relcatRecId.block == -1 && relcatRecId.slot == -1) {
        // (the relation is not found in the Relation Catalog.)
        return E_RELNOTEXIST;
    }

    /* read the record entry corresponding to relcatRecId and create a relCacheEntry
        on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
        update the recId field of this Relation Cache entry to relcatRecId.
        use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
    */
    RecBuffer relCatBlock(relcatRecId.block);
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord, relcatRecId.slot);
    struct RelCacheEntry* relCacheEntry = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry->relCatEntry);
    relCacheEntry->recId = relcatRecId;
    RelCacheTable::relCache[relId] = relCacheEntry;

    /****** Setting up Attribute Cache entry for the relation ******/

    // let listHead be used to hold the head of the linked list of attrCache entries.
    AttrCacheEntry* listHead, *current;

    /*iterate over all the entries in the Attribute Catalog corresponding to each
    attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
    care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
    corresponding to Attribute Catalog before the first call to linearSearch().*/
    RecId attrcatRecId;
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    for(int i = 0; i < relCacheEntry->relCatEntry.numAttrs; i++)
    {
        /* let attrcatRecId store a valid record id an entry of the relation, relName,
        in the Attribute Catalog.*/
        RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, attrRelName, relationName, EQ);

        /* read the record entry corresponding to attrcatRecId and create an
        Attribute Cache entry on it using RecBuffer::getRecord() and
        AttrCacheTable::recordToAttrCatEntry().
        update the recId field of this Attribute Cache entry to attrcatRecId.
        add the Attribute Cache entry to the linked list of listHead .*/
        // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
        RecBuffer attrCatBlock(attrcatRecId.block);
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBlock.getRecord(attrCatRecord, attrcatRecId.slot);
        struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
        attrCacheEntry->recId = attrcatRecId;

        if(i == 0) {
            listHead = attrCacheEntry;
            current = attrCacheEntry;
        }
        else {
            current->next = attrCacheEntry;
            current = attrCacheEntry;
        }
    }
    current->next = nullptr;

    // set the relIdth entry of the AttrCacheTable to listHead.
    AttrCacheTable::attrCache[relId] = listHead;

    /****** Setting up metadata in the Open Relation Table for the relation******/

    // update the relIdth entry of the tableMetaInfo with free as false and
    // relName as the input.
    OpenRelTable::tableMetaInfo[relId].free = false;
    strcpy(OpenRelTable::tableMetaInfo[relId].relName, relName);

    return relId;
}


int OpenRelTable::closeRel(int relId) {
    if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) {
        return E_NOTPERMITTED;
    }

    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    if (tableMetaInfo[relId].free) {
        return E_RELNOTOPEN;
    }

    if(RelCacheTable::relCache[relId]->dirty) {
        Attribute relCatRecord[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&RelCacheTable::relCache[relId]->relCatEntry, relCatRecord);
        RecBuffer relCatBlock(RelCacheTable::relCache[relId]->recId.block);
        relCatBlock.setRecord(relCatRecord, RelCacheTable::relCache[relId]->recId.slot);
    }

    free(RelCacheTable::relCache[relId]);
    RelCacheTable::relCache[relId] = nullptr;

    AttrCacheEntry *entry, *temp;
    entry = AttrCacheTable::attrCache[relId];
    while(entry != nullptr) {
        if(entry->dirty) {
            Attribute record[ATTRCAT_NO_ATTRS];
            AttrCacheTable::attrCatEntryToRecord(&entry->attrCatEntry, record);
            RecBuffer attrCatBlock(entry->recId.block);
            attrCatBlock.setRecord(record, entry->recId.slot);
        }

        temp = entry;
        entry = entry->next;
        free(temp);
    }
    AttrCacheTable::attrCache[relId] = nullptr;

    // update `tableMetaInfo` to set `relId` as a free slot
    tableMetaInfo[relId].free = true;
    
    return SUCCESS;
}
