#include "PltSyncUtils.h"
#include "PltUtilities.h"
#include "PltRelationshipStructures.h"

//See Table B-1 in Appendix B.  of ContentSync:1 Service Template Version 1.01
const char* syncableObjects[]= {"object.item",
                                "object.item.imageItem",
                                "object.item.imageItem.photo",
                                "object.item.audioItem",
                                "object.item.audioItem.musicTrack",
                                "object.item.audioItem.audioBook",
                                "object.item.videoItem",
                                "object.item.videoItem.movie",
                                "object.item.videoItem.musicVideoClip",
                                "object.item.playlistItem",
                                "object.item.bookmarkItem",
                                "object.item.textItem",
                                "object.item.epgItem",
                                "object.item.epgItem.audioProgram",
                                "object.item.epgItem.videoProgram",
                                "object.container",
                                "object.container.person",
                                "object.container.person.musicArtist",
                                "object.container.playlistContainer",
                                "object.container.album",
                                "object.container.album.musicAlbum",
                                "object.container.album.photoAlbum",
                                "object.container.genre",
                                "object.container.genre.musicGenre",
                                "object.container.genre.movieGenre",
                                "object.container.epgContainer",
                                "object.container.bookmarkFolder"
};


bool DeviceUDNFinder::operator()(const PLT_Partner& partner) const
{
  return partner.m_strDeviceUDN == udn;
}

bool IsSyncable(const NPT_String& object_class)
{
  return true; //TODO:
}

bool IsValidUDN(const NPT_String& udn) { return true; /*TODO: */}
bool IsValidID(const NPT_String& id) { return true; /*TODO: */}
