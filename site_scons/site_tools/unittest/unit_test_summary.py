from collections import OrderedDict

from color import ColorString
from prettytable import PrettyTable


class UnitTestSummary(object):
    COLUMNS = ["Name", "Result"]

    RESULT_PASSED = "PASSED"
    RESULT_FAILED = "FAILED"

    def __init__(self):
        self._unit_test_result_map = OrderedDict()

    def __len__(self):
        return len(self._unit_test_result_map)

    def __iter__(self):
        for item in self._unit_test_result_map.items():
            yield item

    def __str__(self):
        prettytable = PrettyTable(field_names=self.COLUMNS, title="Unit Test Operation Summary")
        for row in self:
            prettytable.add_row(row)
        return prettytable.get_string()

    """
    Public methods
    """
    def add_result(self, unit_test_name, success):
        """
        :param unit_test_name: Name of a unit test to display (str)
        :param success: True if the unit test execution is successful (bool)
        """
        result = self.RESULT_PASSED if success else self.RESULT_FAILED
        self._unit_test_result_map[unit_test_name] = self._get_result_string_colored(result)

    """
    Private methods
    """
    def _get_result_string_colored(self, result):
        result_color_map = {
            self.RESULT_PASSED: ColorString(result).green,
            self.RESULT_FAILED: ColorString(result).red,
        }
        return result_color_map[result]

    """
    Accessors
    """
    @property
    def has_failure(self):
        failure = False
        for _, result in self:
            if self.RESULT_FAILED in result:
                failure = True
                break
        return failure
