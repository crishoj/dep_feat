
module Annotation

  def self.find_annotator(kind)
    Annotation.const_get(kind.camelize)
  end

end
