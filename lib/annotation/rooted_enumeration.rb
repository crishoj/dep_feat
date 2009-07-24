
module Annotation
  class RootedEnumeration < Enumeration

    def mark_span(first, last)
      # Mark first item specially
      super(first, last)
      first.prev.features << 'enum-1st'
    end

  end
end
