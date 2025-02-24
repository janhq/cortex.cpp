def assert_equal(actual, expected):
    """Custom assertion to compare actual and expected values."""
    assert actual == expected, f"Assertion failed: Expected '{expected}', but got '{actual}'"
    
def assert_contains(main_string, sub_string):
    """Custom assertion to compare actual and expected values."""
    assert sub_string in main_string, f"Assertion failed: Expected '{main_string}' has '{sub_string}'"
