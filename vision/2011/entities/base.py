
class VisionEntity(object):
    '''Defines an entity, or object that can be located.'''

    # A human readable name for this entity
    name = "VisionEntity"

    # Camera to look through when finding this entity.  Frames from this camera
    # will be passed to VisionEntity.find().  Subclasses MUST specify a camera.
    #
    # The camera name should be a string.
    camera_name = None

    def find(self, frames):
        '''Find the VisionEntity in the given frame.

        Returns true when the entity is seen.  When the entity is seen, find()
        also records information in the object about where the entity was seen.  

        Whenever find() returns True, the entity's object is sent to mission logic.

        '''
        raise NotImplementedError("This subclass must be implemented!")

    def __repr__(self):
        return "<%s>" % self.name