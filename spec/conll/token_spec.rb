
require 'spec/conll/spec_helper'
require 'lib/conll/token'

describe Conll::Token do

  before(:each) do
    @token = Conll::Token.parse(@token_line)
  end

  it "should return a Token for a parsed line" do
    @token.should be_kind_of(Conll::Token)
  end

  it "should have a form" do
    @token.should respond_to(:form)
  end

  it "should have form 'To'" do
    @token.form.should match(/To/)
  end

  it "should have CPOS 'A'" do
    @token.cpos.should match(/^A$/)
  end

  it "should have POS 'AC'" do
    @token.pos.should match(/^AC$/)
  end

  it "should register itself on assignment" do
   sentence = Conll::Sentence.new
   @token.sentence = sentence
   sentence.tokens.last.should equal(@token)
  end

end

